/**
 * @file    json_bytes.ipp
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

namespace rorm::detail {

template<typename Bytes>
std::expected<std::string, std::error_code> encode_base64( const Bytes& value )
{
    constexpr std::string_view alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string                result;
    for ( std::size_t i = 0; i < value.size(); i += 3 )
    {
        const auto a = static_cast<unsigned>( value[i] );
        const auto b = i + 1 < value.size() ? static_cast<unsigned>( value[i + 1] ) : 0;
        const auto c = i + 2 < value.size() ? static_cast<unsigned>( value[i + 2] ) : 0;
        result      += alphabet[a >> 2];
        result      += alphabet[( ( a & 3 ) << 4 ) | ( b >> 4 )];
        result      += i + 1 < value.size() ? alphabet[( ( b & 15 ) << 2 ) | ( c >> 6 )] : '=';
        result      += i + 2 < value.size() ? alphabet[c & 63] : '=';
    }
    return result;
}

template<typename Bytes>
std::expected<Bytes, std::error_code> decode_base64( std::string_view text )
{
    const auto invalid = std::unexpected( make_error_code( json_error_code::incompatible_value ) );
    if ( text.size() % 4 != 0 )
        return invalid;
    const auto digit = []( char c ) -> int {
        if ( c >= 'A' && c <= 'Z' )
            return c - 'A';
        if ( c >= 'a' && c <= 'z' )
            return c - 'a' + 26;
        if ( c >= '0' && c <= '9' )
            return c - '0' + 52;
        if ( c == '+' )
            return 62;
        if ( c == '/' )
            return 63;
        return -1;
    };
    const auto padding = text.empty() ? 0 : ( text.back() == '=' ? ( text[text.size() - 2] == '=' ? 2 : 1 ) : 0 );
    const auto length  = text.size() / 4 * 3 - padding;
    Bytes      result {};
    if constexpr ( sequence_traits<Bytes>::fixed )
    {
        if ( length != result.size() )
            return invalid;
    }
    else
        result.resize( length );
    std::size_t output = 0;
    for ( std::size_t i = 0; i < text.size(); i += 4 )
    {
        const int a = digit( text[i] );
        const int b = digit( text[i + 1] );
        const int c = text[i + 2] == '=' ? 0 : digit( text[i + 2] );
        const int d = text[i + 3] == '=' ? 0 : digit( text[i + 3] );
        if ( a < 0 || b < 0 || c < 0 || d < 0 )
            return invalid;
        const bool pad2 = text[i + 2] == '=';
        const bool pad1 = text[i + 3] == '=';
        if ( ( pad2 || pad1 ) && i + 4 != text.size() )
            return invalid;
        if ( pad2 && ( !pad1 || ( b & 15 ) != 0 ) )
            return invalid;
        if ( pad1 && !pad2 && ( c & 3 ) != 0 )
            return invalid;
        using Element    = typename sequence_traits<Bytes>::element_type;
        result[output++] = static_cast<Element>( ( a << 2 ) | ( b >> 4 ) );
        if ( !pad2 )
            result[output++] = static_cast<Element>( ( b << 4 ) | ( c >> 2 ) );
        if ( !pad1 )
            result[output++] = static_cast<Element>( ( c << 6 ) | d );
    }
    return result;
}
} // namespace rorm::detail
