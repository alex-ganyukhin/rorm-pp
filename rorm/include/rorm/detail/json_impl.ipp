/**
 * @file    json_impl.ipp
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
#include <nlohmann/json.hpp>
#include <rorm/detail/record_mapping.h>

namespace rorm::detail {
using json_value = nlohmann::json;
inline std::expected<json_value, std::error_code> parse_json( std::string_view input );
template<typename T>
std::expected<json_value, std::error_code> encode_value( const T& value, bool base64 );
template<typename T>
std::expected<T, std::error_code> decode_value( const json_value& value, deserialization_policy policy, bool base64 );
template<typename Bytes>
std::expected<std::string, std::error_code> encode_base64( const Bytes& value );
template<typename Bytes>
std::expected<Bytes, std::error_code> decode_base64( std::string_view text );
template<typename Chrono>
std::expected<std::string, std::error_code> format_chrono( const Chrono& value );
template<typename Chrono>
std::expected<Chrono, std::error_code> parse_chrono( std::string_view text );

template<typename Record>
std::expected<json_value, std::error_code> encode_record( const Record& record )
{
    json_value result = json_value::object();
    template for ( constexpr auto p : record_mapping<Record>::properties )
    {
        using P               = mapped_property<Record, p.reader, p.writer>;
        constexpr auto name   = p.name;
        constexpr bool base64 = p.base64;
        if constexpr ( P::readable )
        {
            auto logical = P::read( record );
            if ( !logical )
                return std::unexpected( logical.error() );
            auto encoded = encode_value( *logical, base64 );
            if ( !encoded )
                return std::unexpected( encoded.error() );
            result[std::string( name )] = std::move( *encoded );
        }
    }
    return result;
}

template<typename Record>
std::expected<Record, std::error_code> decode_record( const json_value& input, deserialization_policy policy )
{
    static_assert( sizeof( record_mapping<Record> ) > 0 );
    if ( !input.is_object() )
        return std::unexpected( make_error_code( json_error_code::incompatible_value ) );
    if ( !policy.ignore_unknown_members )
        for ( auto i = input.begin(); i != input.end(); ++i )
        {
            bool known = false;
            template for ( constexpr auto p : record_mapping<Record>::properties )
            {
                constexpr auto name = p.name;
                if ( name == i.key() )
                    known = true;
            }
            if ( !known )
                return std::unexpected( make_error_code( json_error_code::unknown_member ) );
        }
    Record result {};
    template for ( constexpr auto p : record_mapping<Record>::properties )
    {
        using P               = mapped_property<Record, p.reader, p.writer>;
        constexpr auto name   = p.name;
        constexpr bool base64 = p.base64;
        if constexpr ( P::writable )
        {
            using T      = typename P::logical_type;
            const auto i = input.find( std::string( name ) );
            if ( i == input.end() )
            {
                if constexpr ( is_optional<T> )
                {
                    auto assigned = P::write( result, T {} );
                    if ( !assigned )
                        return std::unexpected( assigned.error() );
                }
                else
                    return std::unexpected( make_error_code( json_error_code::missing_property ) );
            }
            else
            {
                auto logical = decode_value<T>( *i, policy, base64 );
                if ( !logical )
                    return std::unexpected( logical.error() );
                auto assigned = P::write( result, std::move( *logical ) );
                if ( !assigned )
                    return std::unexpected( assigned.error() );
            }
        }
    }
    return result;
}
} // namespace rorm::detail

#include "json_bytes.ipp"
#include "json_chrono.ipp"
#include "json_input.ipp"
#include "json_values.ipp"
