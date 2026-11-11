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
#include <limits>

namespace rorm::detail {
template<typename T>
std::expected<json_value, std::error_code> encode_value( const T& value, bool base64 )
{
    if constexpr ( std::is_same_v<T, std::string> || std::is_integral_v<T> )
        return json_value( value );
    else if constexpr ( is_optional<T> )
    {
        if ( !value )
            return json_value( nullptr );
        return encode_value( *value, base64 );
    }
    else if constexpr ( std::is_class_v<T> && !is_sequence<T> && !chrono_traits<T>::value &&
                        !std::is_same_v<T, std::chrono::year_month_day> )
        return encode_record( value );
    else
        return std::unexpected( make_error_code(
                json_error_code::incompatible_value ) ); // Temporary GREEN scaffold; removed by value worker.
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
    else if constexpr ( is_optional<T> )
    {
        if ( value.is_null() )
            return T {};
        auto decoded = decode_value<typename optional_traits<T>::element_type>( value, policy, base64 );
        if ( !decoded )
            return std::unexpected( decoded.error() );
        return T { std::move( *decoded ) };
    }
    else if constexpr ( std::is_class_v<T> && !is_sequence<T> && !chrono_traits<T>::value &&
                        !std::is_same_v<T, std::chrono::year_month_day> )
        return decode_record<T>( value, policy );
    return std::unexpected( make_error_code( json_error_code::incompatible_value ) ); // Temporary GREEN scaffold.
}
} // namespace rorm::detail
