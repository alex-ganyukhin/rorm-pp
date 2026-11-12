/**
 * @file    json_values.ipp
 * Copyright 2026 Aleksandr Ganiukhin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once
#include <cmath>
#include <limits>
#include <ranges>

namespace rorm::detail {
template<typename T>
struct enum_mapping
{
    static constexpr auto enumerators = std::define_static_array( std::meta::enumerators_of( ^^T ) );

    static consteval bool validate()
    {
        for ( std::size_t i = 0; i < enumerators.size(); ++i )
            for ( std::size_t j = 0; j < i; ++j )
                if ( logical_name( enumerators[i] ) == logical_name( enumerators[j] ) )
                    return false;
        return true;
    }

    static_assert( validate(), "rorm: duplicate effective enum name" );
};

template<typename T>
std::expected<json_value, std::error_code> encode_value( const T& value, bool base64 )
{
    if constexpr ( std::is_same_v<T, std::string> || std::is_integral_v<T> )
        return json_value( value );
    else if constexpr ( std::is_same_v<T, std::byte> )
        return json_value( std::to_integer<unsigned int>( value ) );
    else if constexpr ( std::is_floating_point_v<T> )
    {
        if ( !std::isfinite( value ) )
            return std::unexpected( make_error_code( json_error_code::incompatible_value ) );
        return json_value( value );
    }
    else if constexpr ( std::is_enum_v<T> )
    {
        template for ( constexpr auto e : enum_mapping<T>::enumerators )
        {
            constexpr auto name = logical_name( e );
            if ( value == [:e:] )
                return json_value( std::string( name ) );
        }
        return std::unexpected( make_error_code( json_error_code::incompatible_value ) );
    }
    else if constexpr ( is_optional<T> )
    {
        if ( !value )
            return json_value( nullptr );
        return encode_value( *value, base64 );
    }
    else if constexpr ( is_sequence<T> )
    {
        if constexpr ( byte_sequence<T>() )
            if ( base64 )
            {
                auto text = encode_base64( value );
                if ( !text )
                    return std::unexpected( text.error() );
                return json_value( std::move( *text ) );
            }
        auto result = json_value::array();
        for ( const auto& item : value )
        {
            // vector<bool> exposes proxy references rather than bool values.
            auto encoded = encode_value<typename sequence_traits<T>::element_type>( item, false );
            if ( !encoded )
                return std::unexpected( encoded.error() );
            result.push_back( std::move( *encoded ) );
        }
        return result;
    }
    else if constexpr ( chrono_traits<T>::value || std::is_same_v<T, std::chrono::year_month_day> )
    {
        auto text = format_chrono( value );
        if ( !text )
            return std::unexpected( text.error() );
        return json_value( std::move( *text ) );
    }
    else if constexpr ( std::is_class_v<T> && !std::ranges::range<T> )
        return encode_record( value );
    else
        static_assert( dependent_false<T>, "rorm: unsupported JSON value family" );
}

template<typename T>
std::expected<T, std::error_code> decode_value( const json_value& value, deserialization_policy policy, bool base64 )
{
    if constexpr ( std::is_same_v<T, std::string> )
    {
        if ( value.is_string() )
            return value.template get<std::string>();
    }
    else if constexpr ( std::is_same_v<T, bool> )
    {
        if ( value.is_boolean() )
            return value.template get<bool>();
    }
    else if constexpr ( std::is_same_v<T, std::byte> )
    {
        auto decoded = decode_value<unsigned char>( value, policy, false );
        if ( !decoded )
            return std::unexpected( decoded.error() );
        return static_cast<std::byte>( *decoded );
    }
    else if constexpr ( std::is_integral_v<T> )
    {
        if ( value.is_number_integer() )
        {
            if ( value.is_number_unsigned() )
            {
                auto n = value.template get<std::uint64_t>();
                if ( n > static_cast<std::uint64_t>( std::numeric_limits<T>::max() ) )
                    return std::unexpected( make_error_code( json_error_code::out_of_range ) );
                return static_cast<T>( n );
            }
            auto n = value.template get<std::int64_t>();
            if ( std::cmp_less( n, std::numeric_limits<T>::lowest() ) ||
                 std::cmp_greater( n, std::numeric_limits<T>::max() ) )
                return std::unexpected( make_error_code( json_error_code::out_of_range ) );
            return static_cast<T>( n );
        }
    }
    else if constexpr ( std::is_floating_point_v<T> )
    {
        if ( value.is_number() )
        {
            auto n = value.template get<double>();
            if ( !std::isfinite( n ) || n > std::numeric_limits<T>::max() || n < std::numeric_limits<T>::lowest() )
                return std::unexpected( make_error_code( json_error_code::out_of_range ) );
            auto result = static_cast<T>( n );
            if ( n != 0 && result == 0 )
                return std::unexpected( make_error_code( json_error_code::out_of_range ) );
            return result;
        }
    }
    else if constexpr ( std::is_enum_v<T> )
    {
        if ( value.is_string() )
        {
            const auto& text = value.template get_ref<const std::string&>();
            template for ( constexpr auto e : enum_mapping<T>::enumerators )
            {
                constexpr auto name = logical_name( e );
                if ( text == name )
                    return [:e:];
            }
        }
    }
    else if constexpr ( is_optional<T> )
    {
        if ( value.is_null() )
            return T {};
        auto decoded = decode_value<typename optional_traits<T>::element_type>( value, policy, base64 );
        if ( !decoded )
            return std::unexpected( decoded.error() );
        return T { std::move( *decoded ) };
    }
    else if constexpr ( is_sequence<T> )
    {
        if constexpr ( byte_sequence<T>() )
            if ( base64 )
            {
                if ( !value.is_string() )
                    return std::unexpected( make_error_code( json_error_code::incompatible_value ) );
                return decode_base64<T>( value.template get_ref<const std::string&>() );
            }
        if ( !value.is_array() )
            return std::unexpected( make_error_code( json_error_code::incompatible_value ) );
        if constexpr ( sequence_traits<T>::fixed )
            if ( value.size() != sequence_traits<T>::size )
                return std::unexpected( make_error_code( json_error_code::incompatible_value ) );
        T result {};
        if constexpr ( !sequence_traits<T>::fixed )
            result.reserve( value.size() );
        std::size_t index = 0;
        for ( const auto& item : value )
        {
            auto decoded = decode_value<typename sequence_traits<T>::element_type>( item, policy, false );
            if ( !decoded )
                return std::unexpected( decoded.error() );
            if constexpr ( sequence_traits<T>::fixed )
                result[index++] = std::move( *decoded );
            else
                result.push_back( std::move( *decoded ) );
        }
        return result;
    }
    else if constexpr ( chrono_traits<T>::value || std::is_same_v<T, std::chrono::year_month_day> )
    {
        if ( value.is_string() )
            return parse_chrono<T>( value.template get_ref<const std::string&>() );
    }
    else if constexpr ( std::is_class_v<T> && !std::ranges::range<T> )
        return decode_record<T>( value, policy );
    else
        static_assert( dependent_false<T>, "rorm: unsupported JSON value family" );
    return std::unexpected( make_error_code( json_error_code::incompatible_value ) );
}
} // namespace rorm::detail
