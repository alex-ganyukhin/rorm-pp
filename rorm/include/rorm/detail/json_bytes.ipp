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
std::expected<std::string, std::error_code> encode_base64( const Bytes& )
{
    return std::unexpected( make_error_code( json_error_code::incompatible_value ) );
}

template<typename Bytes>
std::expected<Bytes, std::error_code> decode_base64( std::string_view )
{
    return std::unexpected( make_error_code( json_error_code::incompatible_value ) );
}
} // namespace rorm::detail
