/**
 * @file    json.h
 * JSON API/red-test scaffolds. Error-code construction is implemented; Record serialization, parsing, and mapping validation are not.
 * Examples using to_json or from_json illustrate the intended workflow, not working JSON exchange.
 *
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
#ifndef RORM_PP__RORM_JSON_H
#define RORM_PP__RORM_JSON_H

#include <rorm/annotations.h>
#include <rorm/detail/record_mapping.h>

#include <cstddef>
#include <expected>
#include <string>
#include <string_view>
#include <system_error>

namespace rorm {
/**
 * @brief  Choose how deserialization handles members absent from a Record mapping.
 *
 * The policy is required on every from_json call and applies recursively to nested Records. Duplicate members remain errors.
 *
 * @code{.cpp}
 *     struct Profile
 *     {
 *         std::string name;
 *     };
 *     auto strict   = rorm::from_json<Profile>( R"({"name":"Ada"})", {} );
 *     auto tolerant = rorm::from_json<Profile>( R"({"name":"Ada","extra":1})", { .ignore_unknown_members = true } );
 * @endcode
 */
struct deserialization_policy
{
    bool ignore_unknown_members = false; ///< Skip unknown names recursively; duplicates remain errors.
};

/**
 * @brief  Portable JSON failure reasons in the `rorm.json` error category.
 *
 * Construction and comparison work now; JSON scaffolds do not report these failures yet. Converter errors are intended to be forwarded
 * unchanged, so callers must also handle other categories. Paths and byte offsets are outside the first iteration.
 *
 * @code{.cpp}
 *     std::error_code error = rorm::json_error_code::missing_property;
 *     if ( error == rorm::json_error_code::missing_property )
 *     {
 *         std::cerr << error.message() << '\n';
 *     }
 * @endcode
 */
enum class json_error_code
{
    malformed_input = 1, ///< Invalid JSON syntax or encoding.
    duplicate_member,    ///< Repeated object member.
    unknown_member,      ///< Member absent from the mapping under reject policy.
    missing_property,    ///< Missing required writable property.
    incompatible_value,  ///< Value does not match the logical type or representation.
    out_of_range,        ///< Numeric value outside the target type's range.
    conversion_failed ///< Conversion-failure reason; converter-returned errors are intended to pass through unchanged.
};

namespace detail {
    class json_error_category final : public std::error_category
    {
    public:
        const char* name() const noexcept override { return "rorm.json"; }

        std::string message( int value ) const override
        {
            switch ( static_cast<json_error_code>( value ) )
            {
                case json_error_code::malformed_input:    return "Malformed JSON input";
                case json_error_code::duplicate_member:   return "Duplicate member";
                case json_error_code::unknown_member:     return "Unknown member";
                case json_error_code::missing_property:   return "Missing required property";
                case json_error_code::incompatible_value: return "Incompatible value";
                case json_error_code::out_of_range:       return "Value out of range";
                case json_error_code::conversion_failed:  return "Conversion failed";
            }
            return "Unknown JSON error";
        }
    };
} // namespace detail

/**
 * @brief   Construct a JSON error code without allocating.
 *
 * The category has static lifetime and is named `rorm.json`. Calling the returned code's message() may allocate.
 *
 * @code{.cpp}
 *     auto error = rorm::make_error_code( rorm::json_error_code::malformed_input );
 *     std::cerr << error.message() << '\n';
 * @endcode
 *
 * @return  A code holding the reason's integer value and the shared JSON error category.
 */
inline std::error_code make_error_code( json_error_code code ) noexcept
{
    static const detail::json_error_category category;
    return { static_cast<int>( code ), category };
}

} // namespace rorm

namespace std {
template<>
struct is_error_code_enum<rorm::json_error_code> : true_type
{};
} // namespace std

#include <rorm/detail/json_impl.ipp>

namespace rorm {
/**
 * @brief   Serialize readable Record properties to an owning UTF-8 JSON string (planned).
 *
 * API/red-test scaffold: currently returns a successful empty string without reading the Record or validating its mapping.
 * The intended serializer includes fields and getters, omits setter-only properties, and leaves object order and whitespace unspecified.
 * No parsing-backend types appear in this API. The unimplemented value-mapping contract remains in doc/api.md.
 *
 * @code{.cpp}
 *     struct Profile
 *     {
 *         std::string name;
 *     };
 *     auto encoded = rorm::to_json( Profile { "Ada" } );
 *     if ( !encoded )
 *     {
 *         std::cerr << encoded.error().message() << '\n';
 *     }
 *     // Intended: *encoded contains {"name":"Ada"}. Currently it is empty.
 * @endcode
 *
 * @tparam  Record  Type whose public fields and marked accessors define the intended mapping.
 *
 * @return  Currently a successful empty string. Intended failures include incompatible values, numeric range errors, and unchanged
 *          converter errors; invalid mappings are intended to fail at compile time.
 */
template<typename Record>
std::expected<std::string, std::error_code> to_json( const Record& record )
{
    static_assert( sizeof( detail::record_mapping<Record> ) > 0 );
    auto value = detail::encode_record( record );
    if ( !value )
        return std::unexpected( value.error() );
    try
    {
        return value->dump();
    }
    catch ( const detail::json_value::type_error& )
    {
        return std::unexpected( make_error_code( json_error_code::incompatible_value ) );
    }
}

/**
 * @brief   Construct a Record and assign its writable properties from JSON (planned).
 *
 * API/red-test scaffold: currently returns Record {} without inspecting input or policy. It performs no parsing or mapping validation.
 * Input is intended to be borrowed only for the duration of the call; the required policy selects unknown-member handling recursively.
 * Intended behavior: missing writable properties fail unless optional; read-only mapped members are accepted but not assigned.
 * Optional values missing or null become disengaged. No partially populated Record is returned on failure. Duplicate members are errors
 * under either policy, including in nested Records. The remaining unimplemented contract is specified in doc/api.md.
 *
 * @code{.cpp}
 *     struct Profile
 *     {
 *         std::string name;
 *     };
 *     auto decoded = rorm::from_json<Profile>( R"({"name":"Ada"})", {} );
 *     if ( !decoded )
 *     {
 *         std::cerr << decoded.error().message() << '\n';
 *     }
 *     // Intended: decoded->name is "Ada". Currently it is empty.
 * @endcode
 *
 * @tparam  Record  Currently must support Record {}. Intended mapping uses public fields and marked accessors.
 *
 * @return  Currently a successful Record {}. Intended failures include malformed JSON, duplicate or unknown members, missing required
 *          properties, incompatible or out-of-range values, and unchanged converter errors.
 */
template<typename Record>
std::expected<Record, std::error_code> from_json( std::string_view input, deserialization_policy policy )
{
    static_assert( sizeof( detail::record_mapping<Record> ) > 0 );
    auto parsed = detail::parse_json( input );
    if ( !parsed )
        return std::unexpected( parsed.error() );
    return detail::decode_record<Record>( *parsed, policy );
}
} // namespace rorm
#endif
