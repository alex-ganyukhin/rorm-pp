/**
 * @file    test_json.cc
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
#include "matchers.h"
#include "print.h"
#include <compare>
#include <nlohmann/json.hpp>
#include <rorm/rorm.h>

#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace rorm_test {

using Json = nlohmann::json;

class JsonTest : public testing::Test
{};

class JsonErrorsSupportTest : public testing::Test
{};

struct Profile
{
    std::string                name;
    std::optional<std::string> nickname;
    int                        visits {};

    auto operator<=>( const Profile& ) const = default;
};

template<>
inline constexpr bool printable_record<Profile> = true;

TEST_F( JsonTest, GIVEN_public_fields_WHEN_serialized_THEN_all_fields_are_present )
{
    EXPECT_THAT( rorm::to_json( Profile { "Ada", "Countess", 7 } ),
                 ExpectedValue( JsonEq( R"({"name":"Ada","nickname":"Countess","visits":7})" ) ) );
}

TEST_F( JsonTest, GIVEN_reordered_members_WHEN_deserialized_THEN_fields_match )
{
    auto value = rorm::from_json<Profile>( R"({"visits":7,"nickname":"Countess","name":"Ada"})", {} );
    ASSERT_THAT( value, ExpectedValue( testing::_ ) );
    EXPECT_THAT( value, ExpectedValue( testing::Eq( Profile { "Ada", "Countess", 7 } ) ) );
}

TEST_F( JsonTest, WHEN_record_round_trips_THEN_all_fields_are_preserved )
{
    const Profile original { "Ada", "Countess", 7 };
    auto          encoded = rorm::to_json( original );
    ASSERT_THAT( encoded, ExpectedValue( testing::_ ) );
    auto decoded = rorm::from_json<Profile>( *encoded, {} );
    ASSERT_THAT( decoded, ExpectedValue( testing::_ ) );
    EXPECT_THAT( decoded, ExpectedValue( testing::Eq( original ) ) );
}

TEST_F( JsonTest, GIVEN_missing_or_null_optional_WHEN_deserialized_THEN_optional_is_disengaged )
{
    for ( auto input : { R"({"name":"Ada","visits":1})", R"({"name":"Ada","nickname":null,"visits":1})" } )
    {
        SCOPED_TRACE( testing::PrintToString( input ) );
        auto value = rorm::from_json<Profile>( input, {} );
        ASSERT_THAT( value, ExpectedValue( testing::_ ) );
        EXPECT_EQ( value->name, "Ada" );
        EXPECT_FALSE( value->nickname );
    }
    EXPECT_THAT( rorm::to_json( Profile { "Ada", std::nullopt, 1 } ),
                 ExpectedValue( JsonEq( R"({"name":"Ada","nickname":null,"visits":1})" ) ) );
}

TEST_F( JsonTest, GIVEN_ignore_policy_WHEN_unknown_member_is_read_THEN_known_fields_match )
{
    auto value = rorm::from_json<Profile>( R"({"name":"Ada","visits":1,"extra":{"x":[1,2]}})",
                                           { .ignore_unknown_members = true } );
    ASSERT_THAT( value, ExpectedValue( testing::_ ) );
    EXPECT_THAT( value, ExpectedValue( testing::Eq( Profile { "Ada", std::nullopt, 1 } ) ) );
}

struct ErrorCase
{
    const char*           input;
    rorm::json_error_code code;
};

class JsonErrors : public testing::TestWithParam<ErrorCase>
{};

TEST_P( JsonErrors, WHEN_invalid_input_is_read_THEN_expected_error_is_returned )
{
    const auto& test   = GetParam();
    const auto  result = rorm::from_json<Profile>( test.input, {} );
    EXPECT_THAT( result, ExpectedError( testing::Eq( rorm::make_error_code( test.code ) ) ) );
}

INSTANTIATE_TEST_SUITE_P(
        InvalidInput,
        JsonErrors,
        testing::Values(
                ErrorCase { R"({"name":"Ada","visits":1,"extra":2})", rorm::json_error_code::unknown_member },
                ErrorCase { R"({"name":"Ada","name":"Grace","visits":1})", rorm::json_error_code::duplicate_member },
                ErrorCase { R"({"name":"Ada"})", rorm::json_error_code::missing_property },
                ErrorCase { R"({"name":null,"visits":1})", rorm::json_error_code::incompatible_value },
                ErrorCase { R"({"name":"Ada","visits":"1"})", rorm::json_error_code::incompatible_value },
                ErrorCase { R"({"name":"Ada","visits":true})", rorm::json_error_code::incompatible_value },
                ErrorCase { R"({"name":"Ada","visits":1.5})", rorm::json_error_code::incompatible_value },
                ErrorCase { R"({"name":"Ada","visits":2147483648})", rorm::json_error_code::out_of_range },
                ErrorCase { "[]", rorm::json_error_code::incompatible_value },
                ErrorCase { "null", rorm::json_error_code::incompatible_value } ),
        ( []( const testing::TestParamInfo<ErrorCase>& info ) {
            static constexpr const char* names[] = { "UnknownMember",
                                                     "DuplicateMember",
                                                     "MissingProperty",
                                                     "NullRequired",
                                                     "StringNumber",
                                                     "BoolNumber",
                                                     "FractionalInteger",
                                                     "IntegerOverflow",
                                                     "ArrayRoot",
                                                     "NullRoot" };
            return names[info.index];
        } ) );

TEST_F( JsonTest, WHEN_malformed_json_is_read_THEN_malformed_input_is_returned )
{
    for ( auto input :
          { "", "{", R"({"name":"Ada",})", R"({"name":"Ada","visits":1} trailing)", R"({"name":"\uD800","visits":1})" } )
    {
        SCOPED_TRACE( testing::PrintToString( input ) );
        const auto result = rorm::from_json<Profile>( input, {} );
        EXPECT_THAT( result,
                     ExpectedError( testing::Eq( rorm::make_error_code( rorm::json_error_code::malformed_input ) ) ) );
    }
}

TEST_F( JsonTest, GIVEN_ignore_policy_WHEN_duplicate_member_is_read_THEN_duplicate_member_is_returned )
{
    auto result = rorm::from_json<Profile>( R"({"name":"Ada","visits":1,"extra":1,"extra":2})",
                                            { .ignore_unknown_members = true } );
    EXPECT_THAT( result,
                 ExpectedError( testing::Eq( rorm::make_error_code( rorm::json_error_code::duplicate_member ) ) ) );
}

struct Renamed
{
    [[= rorm::name( "display_name" )]] std::string name;
    [[= rorm::ignore]] int                         secret { 42 };

    auto operator<=>( const Renamed& ) const = default;
};

template<>
inline constexpr bool printable_record<Renamed> = true;

TEST_F( JsonTest, GIVEN_Renamed_WHEN_serialized_THEN_json_matches )
{
    EXPECT_THAT( rorm::to_json( Renamed { "Ada", 99 } ), ExpectedValue( JsonEq( R"({"display_name":"Ada"})" ) ) );
}

TEST_F( JsonTest, GIVEN_mapping_annotations_WHEN_deserialized_THEN_renames_and_exclusions_apply )
{
    auto value = rorm::from_json<Renamed>( R"({"display_name":"Ada"})", {} );
    ASSERT_THAT( value, ExpectedValue( testing::_ ) );
    EXPECT_THAT( value, ExpectedValue( testing::Eq( Renamed { "Ada", 42 } ) ) );
}

TEST_F( JsonTest, WHEN_unmapped_cpp_names_are_read_THEN_unknown_member_is_returned )
{
    for ( auto input : { R"({"display_name":"Ada","name":"Grace"})", R"({"display_name":"Ada","secret":99})" } )
    {
        SCOPED_TRACE( testing::PrintToString( input ) );
        auto value = rorm::from_json<Renamed>( input, {} );
        EXPECT_THAT( value,
                     ExpectedError( testing::Eq( rorm::make_error_code( rorm::json_error_code::unknown_member ) ) ) );
    }
}

class Accessors
{
public:
    [[= rorm::getter( "email" )]] const std::string& email() const { return email_; }

    [[= rorm::setter( "email" )]] void email( std::string value ) { email_ = std::move( value ); }

    [[= rorm::getter( "label" )]] std::string label() const { return "<" + email_ + ">"; }

    [[= rorm::setter( "token" )]] void token( std::string value ) { token_ = std::move( value ); }

    const std::string& saved_token() const { return token_; }

private:
    std::string email_;
    std::string token_;

public:
    auto operator<=>( const Accessors& ) const = default;
};

template<>
inline constexpr bool printable_record<Accessors> = true;

TEST_F( JsonTest, WHEN_accessors_are_serialized_THEN_only_readable_properties_are_present )
{
    Accessors value;
    value.email( "ada@example.com" );
    value.token( "secret" );
    EXPECT_THAT( rorm::to_json( value ),
                 ExpectedValue( JsonEq( R"({"email":"ada@example.com","label":"<ada@example.com>"})" ) ) );
}

TEST_F( JsonTest, WHEN_accessors_are_deserialized_THEN_writable_state_matches )
{
    auto value = rorm::from_json<Accessors>( R"({"email":"ada@example.com","token":"secret","label":"ignored"})", {} );
    ASSERT_THAT( value, ExpectedValue( testing::_ ) );
    Accessors expected;
    expected.email( "ada@example.com" );
    expected.token( "secret" );
    EXPECT_THAT( value, ExpectedValue( testing::Eq( expected ) ) );
    EXPECT_EQ( value->label(), "<ada@example.com>" );
}

struct Nested
{
    std::vector<Profile> people;

    auto operator<=>( const Nested& ) const = default;
};

template<>
inline constexpr bool printable_record<Nested> = true;

TEST_F( JsonTest, GIVEN_Nested_WHEN_serialized_THEN_json_matches )
{
    EXPECT_THAT( rorm::to_json( Nested { { { "Ada", std::nullopt, 2 } } } ),
                 ExpectedValue( JsonEq( R"({"people":[{"name":"Ada","nickname":null,"visits":2}]})" ) ) );
}

TEST_F( JsonTest, WHEN_nested_records_are_deserialized_THEN_sequence_matches )
{
    auto value = rorm::from_json<Nested>( R"({"people":[{"name":"Ada","visits":2}]})", {} );
    ASSERT_THAT( value, ExpectedValue( testing::_ ) );
    EXPECT_THAT( value, ExpectedValue( testing::Eq( Nested { { { "Ada", std::nullopt, 2 } } } ) ) );
}

TEST_F( JsonTest, WHEN_nested_required_field_is_missing_THEN_missing_property_is_returned )
{
    auto value = rorm::from_json<Nested>( R"({"people":[{"name":"Ada"}]})", {} );
    EXPECT_THAT( value, ExpectedError( testing::Eq( rorm::make_error_code( rorm::json_error_code::missing_property ) ) ) );
}

struct EscapedName
{
    [[= rorm::name( "a~/b" )]] int value {};

    auto operator<=>( const EscapedName& ) const = default;
};

template<>
inline constexpr bool printable_record<EscapedName> = true;

TEST_F( JsonTest, WHEN_renamed_property_has_wrong_type_THEN_input_is_rejected )
{
    auto value = rorm::from_json<EscapedName>( R"({"a~/b":"wrong"})", {} );
    ASSERT_FALSE( value );
}

struct Text
{
    std::string text;

    auto operator<=>( const Text& ) const = default;
};

template<>
inline constexpr bool printable_record<Text> = true;

TEST_F( JsonTest, GIVEN_Text_WHEN_serialized_THEN_json_matches )
{
    EXPECT_THAT( rorm::to_json( Text { "Ada\n\t\"\\\x01 Ж" } ),
                 ExpectedValue( JsonEq( R"({"text":"Ada\n\t\"\\\u0001 Ж"})" ) ) );
}

TEST_F( JsonTest, WHEN_escaped_text_is_deserialized_THEN_unicode_and_controls_are_preserved )
{
    auto value = rorm::from_json<Text>( R"({"text":"Ada\n\t\"\\\u0001 \u0416"})", {} );
    ASSERT_THAT( value, ExpectedValue( testing::_ ) );
    EXPECT_EQ( value->text, "Ada\n\t\"\\\x01 Ж" );
}

struct Numbers
{
    bool          flag {};
    std::int8_t   small {};
    std::uint64_t large {};
    double        real {};

    auto operator<=>( const Numbers& ) const = default;
};

template<>
inline constexpr bool printable_record<Numbers> = true;

TEST_F( JsonTest, GIVEN_Numbers_WHEN_serialized_THEN_json_matches )
{
    EXPECT_THAT( rorm::to_json( Numbers { true, -128, UINT64_MAX, 1.25 } ),
                 ExpectedValue( JsonEq( R"({"flag":true,"small":-128,"large":18446744073709551615,"real":1.25})" ) ) );
}

TEST_F( JsonTest, WHEN_numeric_boundaries_are_deserialized_THEN_values_match )
{
    auto value = rorm::from_json<Numbers>( R"({"flag":true,"small":-128,"large":18446744073709551615,"real":1.25})", {} );
    ASSERT_THAT( value, ExpectedValue( testing::_ ) );
    EXPECT_THAT( value, ExpectedValue( testing::Eq( Numbers { true, -128, UINT64_MAX, 1.25 } ) ) );
}

template<typename T>
struct NumericValue
{
    T value {};

    auto operator<=>( const NumericValue& ) const = default;
};

template<typename T>
inline constexpr bool printable_record<NumericValue<T>> = true;

template<typename T>
class JsonNumericTypes : public testing::Test
{};

using NumericTypes = testing::
        Types<std::int8_t, std::uint8_t, std::int16_t, std::uint16_t, std::int32_t, std::uint32_t, std::int64_t, std::uint64_t, float, double>;
TYPED_TEST_SUITE( JsonNumericTypes, NumericTypes );

TYPED_TEST( JsonNumericTypes, WHEN_numeric_limits_round_trip_THEN_values_are_preserved )
{
    for ( auto expected : { std::numeric_limits<TypeParam>::lowest(), std::numeric_limits<TypeParam>::max() } )
    {
        SCOPED_TRACE( testing::PrintToString( expected ) );
        // The independent JSON oracle supplies numeric literals without using rorm.
        const auto input = Json {
                { "value",
                 expected }
        }.dump();
        EXPECT_THAT( rorm::to_json( NumericValue<TypeParam> { expected } ), ExpectedValue( JsonEq( input ) ) );
        auto value = rorm::from_json<NumericValue<TypeParam>>( input, {} );
        ASSERT_THAT( value, ExpectedValue( testing::_ ) );
        EXPECT_THAT( value, ExpectedValue( testing::Eq( NumericValue<TypeParam> { expected } ) ) );
    }
}

TEST_F( JsonTest, WHEN_integer_is_out_of_range_THEN_out_of_range_is_returned )
{
    for ( auto input :
          { R"({"flag":true,"small":128,"large":1,"real":1})", R"({"flag":true,"small":0,"large":-1,"real":1})" } )
    {
        SCOPED_TRACE( testing::PrintToString( input ) );
        auto value = rorm::from_json<Numbers>( input, {} );
        EXPECT_THAT( value, ExpectedError( testing::Eq( rorm::make_error_code( rorm::json_error_code::out_of_range ) ) ) );
    }
}

TEST_F( JsonTest, WHEN_nonfinite_number_is_serialized_THEN_incompatible_value_is_returned )
{
    for ( auto real : { std::numeric_limits<double>::infinity(), std::numeric_limits<double>::quiet_NaN() } )
    {
        SCOPED_TRACE( testing::PrintToString( real ) );
        auto value = rorm::to_json( Numbers { true, 1, 2, real } );
        EXPECT_THAT( value,
                     ExpectedError( testing::Eq( rorm::make_error_code( rorm::json_error_code::incompatible_value ) ) ) );
    }
}

enum class Status
{
    pending[[= rorm::name( "waiting" )]],
    active
};

struct EnumRecord
{
    Status status {};

    auto operator<=>( const EnumRecord& ) const = default;
};

template<>
inline constexpr bool printable_record<EnumRecord> = true;

TEST_F( JsonTest, GIVEN_EnumRecord_WHEN_serialized_THEN_json_matches )
{
    EXPECT_THAT( rorm::to_json( EnumRecord { Status::pending } ), ExpectedValue( JsonEq( R"({"status":"waiting"})" ) ) );
}

TEST_F( JsonTest, WHEN_enum_name_is_read_THEN_enumerator_matches )
{
    auto value = rorm::from_json<EnumRecord>( R"({"status":"active"})", {} );
    ASSERT_THAT( value, ExpectedValue( testing::_ ) );
    EXPECT_EQ( value->status, Status::active );
}

TEST_F( JsonTest, WHEN_unknown_enum_name_is_read_THEN_incompatible_value_is_returned )
{
    auto value = rorm::from_json<EnumRecord>( R"({"status":"missing"})", {} );
    EXPECT_THAT( value,
                 ExpectedError( testing::Eq( rorm::make_error_code( rorm::json_error_code::incompatible_value ) ) ) );
}

struct Bytes
{
    std::vector<std::byte>                        raw;
    [[= rorm::base64]] std::vector<unsigned char> encoded;
    std::array<int, 2>                            pair {};

    auto operator<=>( const Bytes& ) const = default;
};

template<>
inline constexpr bool printable_record<Bytes> = true;

TEST_F( JsonTest, GIVEN_Bytes_WHEN_serialized_THEN_json_matches )
{
    EXPECT_THAT( rorm::to_json( Bytes {
                         { std::byte { 0 }, std::byte { 255 } },
                         { 1, 2, 3 },
                         { 4, 5 }
    } ),
                 ExpectedValue( JsonEq( R"({"raw":[0,255],"encoded":"AQID","pair":[4,5]})" ) ) );
}

TEST_F( JsonTest, WHEN_byte_sequences_are_deserialized_THEN_values_match )
{
    auto value = rorm::from_json<Bytes>( R"({"raw":[0,255],"encoded":"AQID","pair":[4,5]})", {} );
    ASSERT_THAT( value, ExpectedValue( testing::_ ) );
    EXPECT_THAT( value,
                 ExpectedValue( testing::Eq( Bytes {
                         { std::byte { 0 }, std::byte { 255 } },
                         { 1, 2, 3 },
                         { 4, 5 }
    } ) ) );
}

TEST_F( JsonTest, WHEN_invalid_base64_is_read_THEN_incompatible_value_is_returned )
{
    auto value = rorm::from_json<Bytes>( R"({"raw":[],"encoded":"???","pair":[1,2]})", {} );
    EXPECT_THAT( value,
                 ExpectedError( testing::Eq( rorm::make_error_code( rorm::json_error_code::incompatible_value ) ) ) );
}

TEST_F( JsonTest, WHEN_array_length_is_wrong_THEN_incompatible_value_is_returned )
{
    auto value = rorm::from_json<Bytes>( R"({"raw":[],"encoded":"","pair":[1]})", {} );
    EXPECT_THAT( value,
                 ExpectedError( testing::Eq( rorm::make_error_code( rorm::json_error_code::incompatible_value ) ) ) );
}

struct Dates
{
    std::chrono::year_month_day                      birthday;
    std::chrono::sys_time<std::chrono::milliseconds> created_at;

    auto operator<=>( const Dates& ) const = default;
};

template<>
inline constexpr bool printable_record<Dates> = true;

TEST_F( JsonTest, GIVEN_Dates_WHEN_serialized_THEN_json_matches )
{
    using namespace std::chrono;
    const auto instant = sys_days { 2026y / September / 20 } + 10h + 30min + 15s + 125ms;
    EXPECT_THAT( rorm::to_json( Dates { 1990y / December / 10, instant } ),
                 ExpectedValue( JsonEq( R"({"birthday":"1990-12-10","created_at":"2026-09-20T10:30:15.125Z"})" ) ) );
}

TEST_F( JsonTest, WHEN_timestamp_has_offset_THEN_deserialization_normalizes_to_utc )
{
    using namespace std::chrono;
    const auto instant = sys_days { 2026y / September / 20 } + 10h + 30min + 15s + 125ms;
    auto value = rorm::from_json<Dates>( R"({"birthday":"1990-12-10","created_at":"2026-09-20T12:30:15.125+02:00"})", {} );
    ASSERT_THAT( value, ExpectedValue( testing::_ ) );
    EXPECT_THAT( value, ExpectedValue( testing::Eq( Dates { 1990y / December / 10, instant } ) ) );
}

TEST_F( JsonTest, WHEN_invalid_date_is_read_THEN_incompatible_value_is_returned )
{
    auto value = rorm::from_json<Dates>( R"({"birthday":"2026-02-30","created_at":"2026-09-20T10:30:15.125Z"})", {} );
    EXPECT_THAT( value,
                 ExpectedError( testing::Eq( rorm::make_error_code( rorm::json_error_code::incompatible_value ) ) ) );
}

struct UserId
{
    int value {};

    auto operator<=>( const UserId& ) const = default;
};

template<>
inline constexpr bool printable_record<UserId> = true;

std::string encode_id( UserId value )
{
    return std::to_string( value.value );
}

std::expected<UserId, std::error_code> decode_id( const std::string& value )
{
    if ( value == "42" )
        return UserId { 42 };
    return std::unexpected( std::make_error_code( std::errc::invalid_argument ) );
}

struct Converted
{
    [[= rorm::as<std::string>( encode_id, decode_id )]] UserId id;

    auto operator<=>( const Converted& ) const = default;
};

template<>
inline constexpr bool printable_record<Converted> = true;

TEST_F( JsonTest, GIVEN_Converted_WHEN_serialized_THEN_json_matches )
{
    EXPECT_THAT( rorm::to_json( Converted { UserId { 42 } } ), ExpectedValue( JsonEq( R"({"id":"42"})" ) ) );
}

TEST_F( JsonTest, WHEN_converted_id_is_read_THEN_custom_conversion_applies )
{
    auto value = rorm::from_json<Converted>( R"({"id":"42"})", {} );
    ASSERT_THAT( value, ExpectedValue( testing::_ ) );
    EXPECT_THAT( value, ExpectedValue( testing::Eq( Converted { UserId { 42 } } ) ) );
}

TEST_F( JsonTest, WHEN_decode_conversion_fails_THEN_original_error_is_returned )
{
    auto value = rorm::from_json<Converted>( R"({"id":"bad"})", {} );
    EXPECT_THAT( value, ExpectedError( testing::Eq( std::make_error_code( std::errc::invalid_argument ) ) ) );
}

class ConvertedAccessors
{
public:
    [[= rorm::getter( "id" )]][[= rorm::as<std::string>( encode_id )]] UserId id() const { return value_; }

    [[= rorm::setter( "id" )]][[= rorm::as<std::string>( decode_id )]] void id( UserId value ) { value_ = value; }

private:
    UserId value_;

public:
    auto operator<=>( const ConvertedAccessors& ) const = default;
};

template<>
inline constexpr bool printable_record<ConvertedAccessors> = true;

TEST_F( JsonTest, WHEN_converted_accessors_round_trip_THEN_id_is_preserved )
{
    ConvertedAccessors original;
    original.id( UserId { 42 } );
    EXPECT_THAT( rorm::to_json( original ), ExpectedValue( JsonEq( R"({"id":"42"})" ) ) );
    auto value = rorm::from_json<ConvertedAccessors>( R"({"id":"42"})", {} );
    ASSERT_THAT( value, ExpectedValue( testing::_ ) );
    EXPECT_THAT( value, ExpectedValue( testing::Eq( original ) ) );
}

struct[[= rorm::table( "profiles" )]] FullRecord
{
    [[= rorm::primary_key]][[= rorm::generated]] std::int64_t  key {};
    [[= rorm::name( "display_name" )]] std::string             name;
    [[= rorm::ignore]] std::string                             secret { "local" };
    [[= rorm::as<std::string>( encode_id, decode_id )]] UserId id;
    std::optional<std::string>                                 nickname;
    Nested                                                     nested;

    [[= rorm::getter( "email" )]] const std::string& email() const { return email_; }

    [[= rorm::setter( "email" )]] void email( std::string value ) { email_ = std::move( value ); }

    [[= rorm::getter( "label" )]] std::string label() const { return name + "!"; }

private:
    std::string email_;

public:
    auto operator<=>( const FullRecord& ) const = default;
};

template<>
inline constexpr bool printable_record<FullRecord> = true;

TEST_F( JsonTest, GIVEN_combined_mapping_WHEN_round_tripped_THEN_state_is_preserved )
{
    FullRecord original;
    original.key      = 17;
    original.name     = "Ada";
    original.id       = UserId { 42 };
    original.nickname = "Countess";
    original.nested.people.push_back( Profile { "Grace", std::nullopt, 3 } );
    original.email( "ada@example.com" );
    const auto expected =
            R"({"key":17,"display_name":"Ada","id":"42","nickname":"Countess","nested":{"people":[{"name":"Grace","nickname":null,"visits":3}]},"email":"ada@example.com","label":"Ada!"})";
    EXPECT_THAT( rorm::to_json( original ), ExpectedValue( JsonEq( expected ) ) );
    auto encoded = rorm::to_json( original );
    ASSERT_THAT( encoded, ExpectedValue( testing::_ ) );
    auto decoded = rorm::from_json<FullRecord>( *encoded, {} );
    ASSERT_THAT( decoded, ExpectedValue( testing::_ ) );
    EXPECT_THAT( decoded, ExpectedValue( testing::Eq( original ) ) );
    EXPECT_THAT( rorm::to_json( *decoded ), ExpectedValue( JsonEq( expected ) ) );
}

TEST_F( JsonTest, WHEN_readonly_property_is_absent_THEN_deserialization_succeeds )
{
    auto value = rorm::from_json<Accessors>( R"({"email":"ada@example.com","token":"secret"})", {} );
    ASSERT_THAT( value, ExpectedValue( testing::_ ) );
    EXPECT_EQ( value->label(), "<ada@example.com>" );
}

TEST_F( JsonTest, WHEN_setteronly_property_is_absent_THEN_missing_property_is_returned )
{
    auto value = rorm::from_json<Accessors>( R"({"email":"ada@example.com"})", {} );
    EXPECT_THAT( value, ExpectedError( testing::Eq( rorm::make_error_code( rorm::json_error_code::missing_property ) ) ) );
}

TEST_F( JsonTest, WHEN_nested_unknown_member_is_read_THEN_selected_policy_applies )
{
    const auto input    = R"({"people":[{"name":"Ada","visits":2,"extra":1}]})";
    auto       rejected = rorm::from_json<Nested>( input, {} );
    EXPECT_THAT( rejected,
                 ExpectedError( testing::Eq( rorm::make_error_code( rorm::json_error_code::unknown_member ) ) ) );
    auto accepted = rorm::from_json<Nested>( input, { .ignore_unknown_members = true } );
    ASSERT_THAT( accepted, ExpectedValue( testing::_ ) );
    ASSERT_EQ( accepted->people.size(), 1U );
    EXPECT_EQ( accepted->people.front().name, "Ada" );
}

std::expected<std::string, std::error_code> encode_failure( UserId )
{
    return std::unexpected( std::make_error_code( std::errc::operation_not_supported ) );
}

struct FailedConversion
{
    [[= rorm::as<std::string>( encode_failure, decode_id )]] UserId id;

    auto operator<=>( const FailedConversion& ) const = default;
};

template<>
inline constexpr bool printable_record<FailedConversion> = true;

TEST_F( JsonTest, WHEN_encode_conversion_fails_THEN_original_error_is_returned )
{
    auto value = rorm::to_json( FailedConversion { UserId { 42 } } );
    EXPECT_THAT( value, ExpectedError( testing::Eq( std::make_error_code( std::errc::operation_not_supported ) ) ) );
}

template<typename Record>
concept accepts_implicit_unknown_policy = requires( std::string_view input ) { rorm::from_json<Record>( input ); };
static_assert( !accepts_implicit_unknown_policy<Profile> );
static_assert( std::is_same_v<decltype( rorm::to_json( Profile {} ) ), std::expected<std::string, std::error_code>> );
static_assert( std::is_same_v<decltype( rorm::from_json<Profile>( "", {} ) ), std::expected<Profile, std::error_code>> );

TEST_F( JsonErrorsSupportTest, WHEN_error_codes_are_constructed_THEN_category_and_messages_are_stable )
{
    static_assert( noexcept( rorm::make_error_code( rorm::json_error_code::malformed_input ) ) );
    for ( auto reason : { rorm::json_error_code::malformed_input,
                          rorm::json_error_code::duplicate_member,
                          rorm::json_error_code::unknown_member,
                          rorm::json_error_code::missing_property,
                          rorm::json_error_code::incompatible_value,
                          rorm::json_error_code::out_of_range,
                          rorm::json_error_code::conversion_failed } )
    {
        SCOPED_TRACE( testing::PrintToString( reason ) );
        auto code = rorm::make_error_code( reason );
        EXPECT_TRUE( code );
        EXPECT_EQ( &code.category(), &rorm::make_error_code( reason ).category() );
        EXPECT_FALSE( code.message().empty() );
    }
}
} // namespace rorm_test
