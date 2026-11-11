/**
 * @file    json_input.ipp
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

#include <set>
#include <vector>

namespace rorm::detail {
class json_input_validator final : public nlohmann::json_sax<json_value>
{
public:
    std::error_code error;

    bool null() override { return true; }

    bool boolean( bool ) override { return true; }

    bool number_integer( number_integer_t ) override { return true; }

    bool number_unsigned( number_unsigned_t ) override { return true; }

    bool number_float( number_float_t value, const string_t& token ) override
    {
        if ( token.find_first_of( ".eE" ) == string_t::npos )
            return fail( json_error_code::out_of_range );
        if ( value == 0 )
        {
            const auto mantissa = std::string_view( token ).substr( 0, token.find_first_of( "eE" ) );
            if ( mantissa.find_first_of( "123456789" ) != std::string_view::npos )
                return fail( json_error_code::out_of_range );
        }
        return true;
    }

    bool string( string_t& ) override { return true; }

    bool binary( binary_t& ) override { return true; }

    bool start_object( std::size_t ) override
    {
        object_keys.emplace_back();
        return true;
    }

    bool key( string_t& key ) override
    {
        return object_keys.back().insert( key ).second || fail( json_error_code::duplicate_member );
    }

    bool end_object() override
    {
        object_keys.pop_back();
        return true;
    }

    bool start_array( std::size_t ) override { return true; }

    bool end_array() override { return true; }

    bool parse_error( std::size_t, const std::string&, const json_value::exception& exception ) override
    {
        return fail( exception.id == 406 ? json_error_code::out_of_range : json_error_code::malformed_input );
    }

private:
    std::vector<std::set<std::string>> object_keys;

    bool fail( json_error_code reason )
    {
        error = make_error_code( reason );
        return false;
    }
};

inline std::expected<json_value, std::error_code> parse_json( std::string_view input )
{
    // The backend treats raw NUL as end-of-input; JSON only permits escaped NUL.
    if ( input.find( '\0' ) != std::string_view::npos )
        return std::unexpected( make_error_code( json_error_code::malformed_input ) );
    json_input_validator validator;
    if ( !json_value::sax_parse( input.begin(), input.end(), &validator ) )
        return std::unexpected( validator.error );
    try
    {
        return json_value::parse( input.begin(), input.end() );
    }
    catch ( const json_value::parse_error& )
    {
        return std::unexpected( make_error_code( json_error_code::malformed_input ) );
    }
    catch ( const json_value::out_of_range& )
    {
        return std::unexpected( make_error_code( json_error_code::out_of_range ) );
    }
}
} // namespace rorm::detail
