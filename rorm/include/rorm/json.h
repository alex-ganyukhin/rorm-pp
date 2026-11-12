/**
 * @file    json.h
 * Reflection-derived Record serialization and deserialization.
 *
 * Supported logical values are booleans, integral and floating-point numbers, std::string, enums, std::optional, std::vector,
 * std::array, std::byte, nested Records, std::chrono::year_month_day, and std::chrono::sys_time<Duration>. Other range families and
 * unsupported scalar types fail at compile time; use rorm::as to expose a supported logical type.
 *
 * Strings use UTF-8. Enums use reflected names or rorm::name overrides. Disengaged optionals use null. Sequences preserve element
 * order; fixed arrays require an exact input length. Byte sequences use integer arrays unless annotated with rorm::base64.
 * Boolean, string and numeric kinds are not coerced into one another. Integer input must be an integer token within the target range.
 * Floating-point input accepts numeric tokens with normal rounding; overflow and underflow to zero return out_of_range. Output uses the
 * backend's double representation, with the same range checks; non-finite values return incompatible_value. Use rorm::as for stricter
 * numeric conversion.
 *
 * Calendar dates use valid four-digit YYYY-MM-DD values. Timestamps accept YYYY-MM-DDTHH:MM:SS, an optional nonempty decimal fraction,
 * and a required Z or signed HH:MM offset. Invalid calendar/time fields, leap seconds and trailing text are rejected. Input offsets
 * normalize to UTC before rounding directly to the destination duration: nearest tick, with an exact tie selecting the even
 * epoch-relative tick. Arbitrarily many fractional digits are accepted without a floating-point or nanosecond intermediate.
 * Timestamp representations must be integral, at most 64 bits, with a positive std::ratio period. A valid instant outside years
 * 0000 through 9999 or the destination representation returns out_of_range, including rounding overflow.
 *
 * Timestamp output uses UTC and Z. Finite decimal periods emit the width derived from the reduced period's denominator (milliseconds,
 * microseconds and nanoseconds retain 3, 6 and 9 digits). Repeating periods truncate to 20 fractional digits deterministically; that
 * approximation decodes to the same destination tick for the supported representations and periods.
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
 * Converter errors pass through unchanged, so callers must also handle other categories. Paths and byte offsets are not supplied.
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
    conversion_failed    ///< Conversion-failure reason; converter-returned errors pass through unchanged.
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
 * @brief   Serialize readable Record properties to an owning UTF-8 JSON string.
 *
 * Includes public fields and getters, omits ignored fields and setter-only properties. ORM key/generated tags do not remove JSON
 * properties. Logical values are copied before encoding; the Record is not modified. Object order and whitespace are unspecified.
 * Invalid mappings fail at compile time from either JSON operation. See the file documentation for supported value representations.
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
 *     // On success, *encoded contains {"name":"Ada"}.
 * @endcode
 *
 * @tparam  Record  Class whose public fields and annotated accessors define a valid mapping.
 *
 * @throws  std::bad_alloc  Allocation failure; exceptions from user getters and converters also propagate unchanged.
 *
 * @return  Owning JSON text, or incompatible_value for invalid representations (including invalid UTF-8 and non-finite numbers),
 *          out_of_range for numeric/timestamp range failure, or an unchanged converter error.
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
 * @brief   Construct a Record and assign its writable properties from a JSON object.
 *
 * Borrows input only during this call; returned strings and values own their storage. The required policy applies recursively.
 * Nonoptional writable properties are required even when their fields have initializers. Missing or null optional logical values
 * become disengaged. Getter-only names are known and accepted without decoding or assignment; their subtrees still undergo syntax,
 * encoding, duplicate-key and numeric-token validation. Setter-only properties participate in the same required/optional rules.
 *
 * The entire input is validated before assignment. Duplicate decoded member names fail under either policy, even in ignored or
 * read-only subtrees. Trailing input and malformed UTF-8 are rejected. On failure no partially populated Record is returned, although
 * user constructors, setters and converters may already have produced external side effects. Invalid mappings fail at compile time.
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
 *     // On success, decoded->name is "Ada".
 * @endcode
 *
 * @tparam  Record  Value-initializable class with a valid writable mapping.
 *
 * @param   input   Complete JSON text; the root must be an object.
 * @param   policy  Whether unknown names are ignored; the default field value rejects them.
 *
 * @throws  std::bad_alloc  Allocation failure; exceptions from user construction, setters and converters propagate unchanged.
 *
 * @return  Owning Record, or malformed_input, duplicate_member, unknown_member, missing_property, incompatible_value,
 *          out_of_range, or an unchanged converter error. Representation rules are documented at file scope.
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
