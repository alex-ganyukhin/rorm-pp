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
#include <gtest/gtest.h>
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

namespace {
using rorm::unknown_members;
using Json = nlohmann::json;

// Compare JSON values, never the backend's formatting or object-member order.
template<typename Record>
void expect_json( const Record& record, std::string_view expected )
{
    const auto encoded = rorm::to_json( record );
    ASSERT_TRUE( encoded.has_value() );
    const auto actual = Json::parse( *encoded, nullptr, false );
    ASSERT_FALSE( actual.is_discarded() ) << *encoded;
    EXPECT_EQ( actual, Json::parse( expected ) );
}

struct Profile
{
    std::string                name;
    std::optional<std::string> nickname;
    int                        visits {};
};

TEST( Json, SerializesPublicFields )
{
    expect_json( Profile { "Ada", "Countess", 7 }, R"({"name":"Ada","nickname":"Countess","visits":7})" );
}

TEST( Json, DeserializesPublicFields )
{
    auto value = rorm::from_json<Profile>( R"({"visits":7,"nickname":"Countess","name":"Ada"})", unknown_members::reject );
    ASSERT_TRUE( value );
    EXPECT_EQ( value->name, "Ada" );
    EXPECT_EQ( value->nickname, "Countess" );
    EXPECT_EQ( value->visits, 7 );
}

TEST( Json, RoundTripsWithoutDataLoss )
{
    const Profile original { "Ada", "Countess", 7 };
    auto          encoded = rorm::to_json( original );
    ASSERT_TRUE( encoded );
    auto decoded = rorm::from_json<Profile>( *encoded, unknown_members::reject );
    ASSERT_TRUE( decoded );
    EXPECT_EQ( decoded->name, original.name );
    EXPECT_EQ( decoded->nickname, original.nickname );
    EXPECT_EQ( decoded->visits, original.visits );
}

TEST( Json, MissingAndNullOptionalValuesAreDisengaged )
{
    for ( auto input : { R"({"name":"Ada","visits":1})", R"({"name":"Ada","nickname":null,"visits":1})" } )
    {
        auto value = rorm::from_json<Profile>( input, unknown_members::reject );
        ASSERT_TRUE( value );
        EXPECT_EQ( value->name, "Ada" );
        EXPECT_FALSE( value->nickname );
    }
    expect_json( Profile { "Ada", std::nullopt, 1 }, R"({"name":"Ada","nickname":null,"visits":1})" );
}

TEST( Json, IgnoresUnknownMembersOnlyWhenRequested )
{
    auto value = rorm::from_json<Profile>( R"({"name":"Ada","visits":1,"extra":{"x":[1,2]}})", unknown_members::ignore );
    ASSERT_TRUE( value );
    EXPECT_EQ( value->name, "Ada" );
    EXPECT_EQ( value->visits, 1 );
}

struct ErrorCase
{
    const char*           input;
    rorm::json_error_code code;
};

class JsonErrors : public testing::TestWithParam<ErrorCase>
{};

TEST_P( JsonErrors, ReportsReasonCode )
{
    const auto& test   = GetParam();
    const auto  result = rorm::from_json<Profile>( test.input, unknown_members::reject );
    ASSERT_FALSE( result.has_value() );
    EXPECT_EQ( result.error(), rorm::make_error_code( test.code ) );
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
                ErrorCase { "null", rorm::json_error_code::incompatible_value } ) );

TEST( Json, ReportsMalformedInput )
{
    for ( auto input :
          { "", "{", R"({"name":"Ada",})", R"({"name":"Ada","visits":1} trailing)", R"({"name":"\uD800","visits":1})" } )
    {
        const auto result = rorm::from_json<Profile>( input, unknown_members::reject );
        ASSERT_FALSE( result );
        EXPECT_EQ( result.error(), rorm::make_error_code( rorm::json_error_code::malformed_input ) );
    }
}

TEST( Json, RejectsDuplicateMembersEvenWhenUnknownMembersAreIgnored )
{
    auto result = rorm::from_json<Profile>( R"({"name":"Ada","visits":1,"extra":1,"extra":2})", unknown_members::ignore );
    ASSERT_FALSE( result );
    EXPECT_EQ( result.error(), rorm::make_error_code( rorm::json_error_code::duplicate_member ) );
}

struct Renamed
{
    [[= rorm::name( "display_name" )]] std::string name;
    [[= rorm::ignore]] int                         secret { 42 };
};

TEST( Json, AppliesRenamesAndExclusionsInBothDirections )
{
    expect_json( Renamed { "Ada", 99 }, R"({"display_name":"Ada"})" );
    auto value = rorm::from_json<Renamed>( R"({"display_name":"Ada"})", unknown_members::reject );
    ASSERT_TRUE( value );
    EXPECT_EQ( value->name, "Ada" );
    EXPECT_EQ( value->secret, 42 );
}

TEST( Json, UnmappedCppNamesAreUnknown )
{
    for ( auto input : { R"({"display_name":"Ada","name":"Grace"})", R"({"display_name":"Ada","secret":99})" } )
    {
        auto value = rorm::from_json<Renamed>( input, unknown_members::reject );
        ASSERT_FALSE( value );
        EXPECT_EQ( value.error(), rorm::make_error_code( rorm::json_error_code::unknown_member ) );
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
};

TEST( Json, SerializesOnlyReadableProperties )
{
    Accessors value;
    value.email( "ada@example.com" );
    value.token( "secret" );
    expect_json( value, R"({"email":"ada@example.com","label":"<ada@example.com>"})" );
}

TEST( Json, AssignsWritablePropertiesAndAcceptsReadOnlyInput )
{
    auto value = rorm::from_json<Accessors>( R"({"email":"ada@example.com","token":"secret","label":"ignored"})",
                                             unknown_members::reject );
    ASSERT_TRUE( value );
    EXPECT_EQ( value->email(), "ada@example.com" );
    EXPECT_EQ( value->saved_token(), "secret" );
    EXPECT_EQ( value->label(), "<ada@example.com>" );
}

struct Nested
{
    std::vector<Profile> people;
};

TEST( Json, MapsNestedRecordsAndSequences )
{
    expect_json( Nested { { { "Ada", std::nullopt, 2 } } }, R"({"people":[{"name":"Ada","nickname":null,"visits":2}]})" );
    auto value = rorm::from_json<Nested>( R"({"people":[{"name":"Ada","visits":2}]})", unknown_members::reject );
    ASSERT_TRUE( value );
    ASSERT_EQ( value->people.size(), 1U );
    EXPECT_EQ( value->people[0].name, "Ada" );
    EXPECT_EQ( value->people[0].visits, 2 );
}

TEST( Json, ReportsNestedArrayErrors )
{
    auto value = rorm::from_json<Nested>( R"({"people":[{"name":"Ada"}]})", unknown_members::reject );
    ASSERT_FALSE( value );
    EXPECT_EQ( value.error(), rorm::make_error_code( rorm::json_error_code::missing_property ) );
}

struct EscapedName
{
    [[= rorm::name( "a~/b" )]] int value {};
};

TEST( Json, RejectsIncompatibleRenamedProperty )
{
    auto value = rorm::from_json<EscapedName>( R"({"a~/b":"wrong"})", unknown_members::reject );
    ASSERT_FALSE( value );
}

struct Text
{
    std::string text;
};

TEST( Json, PreservesUnicodeAndEscapesControlCharacters )
{
    expect_json( Text { "Ada\n\t\"\\\x01 Ж" }, R"({"text":"Ada\n\t\"\\\u0001 Ж"})" );
    auto value = rorm::from_json<Text>( R"({"text":"Ada\n\t\"\\\u0001 \u0416"})", unknown_members::reject );
    ASSERT_TRUE( value );
    EXPECT_EQ( value->text, "Ada\n\t\"\\\x01 Ж" );
}

struct Numbers
{
    bool          flag {};
    std::int8_t   small {};
    std::uint64_t large {};
    double        real {};
};

TEST( Json, PreservesNumericTypesAndBoundaries )
{
    expect_json( Numbers { true, -128, UINT64_MAX, 1.25 },
                 R"({"flag":true,"small":-128,"large":18446744073709551615,"real":1.25})" );
    auto value = rorm::from_json<Numbers>( R"({"flag":true,"small":-128,"large":18446744073709551615,"real":1.25})",
                                           unknown_members::reject );
    ASSERT_TRUE( value );
    EXPECT_TRUE( value->flag );
    EXPECT_EQ( value->small, -128 );
    EXPECT_EQ( value->large, UINT64_MAX );
    EXPECT_DOUBLE_EQ( value->real, 1.25 );
}

template<typename T>
struct NumericValue
{
    T value {};
};

template<typename T>
class JsonNumericTypes : public testing::Test
{};

using NumericTypes = testing::
        Types<std::int8_t, std::uint8_t, std::int16_t, std::uint16_t, std::int32_t, std::uint32_t, std::int64_t, std::uint64_t, float, double>;
TYPED_TEST_SUITE( JsonNumericTypes, NumericTypes );

TYPED_TEST( JsonNumericTypes, PreservesLowerAndUpperBoundaries )
{
    for ( auto expected : { std::numeric_limits<TypeParam>::lowest(), std::numeric_limits<TypeParam>::max() } )
    {
        // The independent JSON oracle supplies numeric literals without using rorm.
        const auto input = Json {
                { "value",
                 expected }
        }.dump();
        expect_json( NumericValue<TypeParam> { expected }, input );
        auto value = rorm::from_json<NumericValue<TypeParam>>( input, unknown_members::reject );
        ASSERT_TRUE( value );
        EXPECT_EQ( value->value, expected );
    }
}

TEST( Json, RejectsUnsignedUnderflowAndNarrowing )
{
    for ( auto input :
          { R"({"flag":true,"small":128,"large":1,"real":1})", R"({"flag":true,"small":0,"large":-1,"real":1})" } )
    {
        auto value = rorm::from_json<Numbers>( input, unknown_members::reject );
        ASSERT_FALSE( value );
        EXPECT_EQ( value.error(), rorm::make_error_code( rorm::json_error_code::out_of_range ) );
    }
}

TEST( Json, RejectsNonFiniteFloatingPointOutput )
{
    for ( auto real : { std::numeric_limits<double>::infinity(), std::numeric_limits<double>::quiet_NaN() } )
    {
        auto value = rorm::to_json( Numbers { true, 1, 2, real } );
        ASSERT_FALSE( value );
        EXPECT_EQ( value.error(), rorm::make_error_code( rorm::json_error_code::incompatible_value ) );
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
};

TEST( Json, UsesMappedEnumeratorNames )
{
    expect_json( EnumRecord { Status::pending }, R"({"status":"waiting"})" );
    auto value = rorm::from_json<EnumRecord>( R"({"status":"active"})", unknown_members::reject );
    ASSERT_TRUE( value );
    EXPECT_EQ( value->status, Status::active );
}

TEST( Json, RejectsUnknownEnumNames )
{
    auto value = rorm::from_json<EnumRecord>( R"({"status":"missing"})", unknown_members::reject );
    ASSERT_FALSE( value );
    EXPECT_EQ( value.error(), rorm::make_error_code( rorm::json_error_code::incompatible_value ) );
}

struct Bytes
{
    std::vector<std::byte>                        raw;
    [[= rorm::base64]] std::vector<unsigned char> encoded;
    std::array<int, 2>                            pair {};
};

TEST( Json, MapsBytesAndFixedSizeSequences )
{
    expect_json(
            Bytes {
                    { std::byte { 0 }, std::byte { 255 } },
                    { 1, 2, 3 },
                    { 4, 5 }
    },
            R"({"raw":[0,255],"encoded":"AQID","pair":[4,5]})" );
    auto value = rorm::from_json<Bytes>( R"({"raw":[0,255],"encoded":"AQID","pair":[4,5]})", unknown_members::reject );
    ASSERT_TRUE( value );
    EXPECT_EQ( value->raw, ( std::vector<std::byte> { std::byte { 0 }, std::byte { 255 } } ) );
    EXPECT_EQ( value->encoded, ( std::vector<unsigned char> { 1, 2, 3 } ) );
    EXPECT_EQ( value->pair, ( std::array<int, 2> { 4, 5 } ) );
}

TEST( Json, RejectsInvalidBase64 )
{
    auto value = rorm::from_json<Bytes>( R"({"raw":[],"encoded":"???","pair":[1,2]})", unknown_members::reject );
    ASSERT_FALSE( value );
    EXPECT_EQ( value.error(), rorm::make_error_code( rorm::json_error_code::incompatible_value ) );
}

TEST( Json, RejectsWrongFixedSizeSequenceLength )
{
    auto value = rorm::from_json<Bytes>( R"({"raw":[],"encoded":"","pair":[1]})", unknown_members::reject );
    ASSERT_FALSE( value );
    EXPECT_EQ( value.error(), rorm::make_error_code( rorm::json_error_code::incompatible_value ) );
}

struct Dates
{
    std::chrono::year_month_day                      birthday;
    std::chrono::sys_time<std::chrono::milliseconds> created_at;
};

TEST( Json, MapsDatesAndNormalizesTimestampOffsetsToUtc )
{
    using namespace std::chrono;
    const auto instant = sys_days { 2026y / September / 20 } + 10h + 30min + 15s + 125ms;
    expect_json( Dates { 1990y / December / 10, instant },
                 R"({"birthday":"1990-12-10","created_at":"2026-09-20T10:30:15.125Z"})" );
    auto value = rorm::from_json<Dates>( R"({"birthday":"1990-12-10","created_at":"2026-09-20T12:30:15.125+02:00"})",
                                         unknown_members::reject );
    ASSERT_TRUE( value );
    EXPECT_EQ( value->birthday, 1990y / December / 10 );
    EXPECT_EQ( value->created_at, instant );
}

TEST( Json, RejectsInvalidCalendarDates )
{
    auto value = rorm::from_json<Dates>( R"({"birthday":"2026-02-30","created_at":"2026-09-20T10:30:15.125Z"})",
                                         unknown_members::reject );
    ASSERT_FALSE( value );
    EXPECT_EQ( value.error(), rorm::make_error_code( rorm::json_error_code::incompatible_value ) );
}

struct UserId
{
    int value {};
};

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
};

TEST( Json, AppliesCustomConversionsInBothDirections )
{
    expect_json( Converted { UserId { 42 } }, R"({"id":"42"})" );
    auto value = rorm::from_json<Converted>( R"({"id":"42"})", unknown_members::reject );
    ASSERT_TRUE( value );
    EXPECT_EQ( value->id.value, 42 );
}

TEST( Json, ReportsConversionFailure )
{
    auto value = rorm::from_json<Converted>( R"({"id":"bad"})", unknown_members::reject );
    ASSERT_FALSE( value );
    EXPECT_EQ( value.error(), std::make_error_code( std::errc::invalid_argument ) );
}

class ConvertedAccessors
{
public:
    [[= rorm::getter( "id" )]][[= rorm::as<std::string>( encode_id )]] UserId id() const { return value_; }

    [[= rorm::setter( "id" )]][[= rorm::as<std::string>( decode_id )]] void id( UserId value ) { value_ = value; }

private:
    UserId value_;
};

TEST( Json, ConvertsAccessorsInTheirAvailableDirection )
{
    ConvertedAccessors original;
    original.id( UserId { 42 } );
    expect_json( original, R"({"id":"42"})" );
    auto value = rorm::from_json<ConvertedAccessors>( R"({"id":"42"})", unknown_members::reject );
    ASSERT_TRUE( value );
    EXPECT_EQ( value->id().value, 42 );
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
};

TEST( Json, RoundTripsCombinedMappingAndKeepsOrmAnnotationsJsonNeutral )
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
    expect_json( original, expected );
    auto encoded = rorm::to_json( original );
    ASSERT_TRUE( encoded );
    auto decoded = rorm::from_json<FullRecord>( *encoded, unknown_members::reject );
    ASSERT_TRUE( decoded );
    EXPECT_EQ( decoded->key, 17 );
    EXPECT_EQ( decoded->name, "Ada" );
    EXPECT_EQ( decoded->id.value, 42 );
    EXPECT_EQ( decoded->nickname, "Countess" );
    EXPECT_EQ( decoded->secret, "local" );
    EXPECT_EQ( decoded->email(), "ada@example.com" );
    ASSERT_EQ( decoded->nested.people.size(), 1U );
    EXPECT_EQ( decoded->nested.people.front().name, "Grace" );
    expect_json( *decoded, expected );
}

TEST( Json, DoesNotRequireReadOnlyPropertiesOnInput )
{
    auto value = rorm::from_json<Accessors>( R"({"email":"ada@example.com","token":"secret"})", unknown_members::reject );
    ASSERT_TRUE( value );
    EXPECT_EQ( value->label(), "<ada@example.com>" );
}

TEST( Json, RequiresSetterOnlyPropertiesOnInput )
{
    auto value = rorm::from_json<Accessors>( R"({"email":"ada@example.com"})", unknown_members::reject );
    ASSERT_FALSE( value );
    EXPECT_EQ( value.error(), rorm::make_error_code( rorm::json_error_code::missing_property ) );
}

TEST( Json, PropagatesUnknownMemberPolicyIntoNestedRecords )
{
    const auto input    = R"({"people":[{"name":"Ada","visits":2,"extra":1}]})";
    auto       rejected = rorm::from_json<Nested>( input, unknown_members::reject );
    ASSERT_FALSE( rejected );
    EXPECT_EQ( rejected.error(), rorm::make_error_code( rorm::json_error_code::unknown_member ) );
    auto accepted = rorm::from_json<Nested>( input, unknown_members::ignore );
    ASSERT_TRUE( accepted );
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
};

TEST( Json, ReportsSerializationConversionFailures )
{
    auto value = rorm::to_json( FailedConversion { UserId { 42 } } );
    ASSERT_FALSE( value );
    EXPECT_EQ( value.error(), std::make_error_code( std::errc::operation_not_supported ) );
}

template<typename Record>
concept accepts_implicit_unknown_policy = requires( std::string_view input ) { rorm::from_json<Record>( input ); };
static_assert( !accepts_implicit_unknown_policy<Profile> );
static_assert( std::is_same_v<decltype( rorm::to_json( Profile {} ) ), std::expected<std::string, std::error_code>> );
static_assert( std::is_same_v<decltype( rorm::from_json<Profile>( "", unknown_members::reject ) ),
                              std::expected<Profile, std::error_code>> );

TEST( JsonErrorsSupport, ConstructsStableReasonCodes )
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
        auto code = rorm::make_error_code( reason );
        EXPECT_TRUE( code );
        EXPECT_EQ( &code.category(), &rorm::make_error_code( reason ).category() );
        EXPECT_FALSE( code.message().empty() );
    }
}
} // namespace
