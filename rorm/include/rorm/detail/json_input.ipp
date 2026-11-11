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

namespace rorm::detail {
inline std::expected<json_value, std::error_code> parse_json( std::string_view input )
{
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
