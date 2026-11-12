/**
 * @file    test_json_codecs.cc
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
#include <array>
#include <gtest/gtest.h>
#include <limits>
#include <rorm/json.h>
#include <vector>

namespace {
class JsonCodecs : public ::testing::Test
{};

TEST_F( JsonCodecs, WHEN_base64_payloads_are_encoded_THEN_padding_matches )
{
    for ( const auto& [bytes, text] : std::vector<std::pair<std::vector<unsigned char>, std::string>> {
                  {{},                ""    },
                  {{ 102 },           "Zg=="},
                  {{ 102, 111 },      "Zm8="},
                  {{ 102, 111, 111 }, "Zm9v"}
    } )
    {
        SCOPED_TRACE( text );
        EXPECT_EQ( rorm::detail::encode_base64( bytes ), text );
        EXPECT_EQ( rorm::detail::decode_base64<std::vector<unsigned char>>( text ), bytes );
    }
    EXPECT_EQ( ( rorm::detail::decode_base64<std::array<std::byte, 1>>( "Zg==" ) ), ( std::array { std::byte { 102 } } ) );
}

TEST_F( JsonCodecs, WHEN_base64_is_malformed_THEN_incompatible_value_is_returned )
{
    for ( const auto text : { "A", "AA", "AAA", "A===", "=AAA", "AA=A", "AA==AAAA", "AB==", "AAB=", "AA?=", " AA=" } )
    {
        SCOPED_TRACE( text );
        const auto value = rorm::detail::decode_base64<std::vector<unsigned char>>( text );
        ASSERT_FALSE( value );
        EXPECT_EQ( value.error(), rorm::make_error_code( rorm::json_error_code::incompatible_value ) );
    }
    EXPECT_FALSE( ( rorm::detail::decode_base64<std::array<unsigned char, 2>>( "Zg==" ) ) );
}

TEST_F( JsonCodecs, WHEN_calendar_dates_are_converted_THEN_leap_days_are_validated )
{
    using namespace std::chrono;
    EXPECT_EQ( rorm::detail::parse_chrono<year_month_day>( "2024-02-29" ), year_month_day( 2024y / February / 29 ) );
    EXPECT_FALSE( rorm::detail::parse_chrono<year_month_day>( "2023-02-29" ) );
    EXPECT_FALSE( rorm::detail::format_chrono( year_month_day( 2023y / February / 29 ) ) );
    EXPECT_EQ( rorm::detail::format_chrono( year_month_day( 2024y / February / 29 ) ), "2024-02-29" );
}

template<typename Duration>
auto parse_ticks( std::string_view text )
{
    return rorm::detail::parse_chrono<std::chrono::sys_time<Duration>>( text ).transform(
            []( auto time ) { return time.time_since_epoch().count(); } );
}

TEST_F( JsonCodecs, WHEN_timestamp_fractions_are_rounded_THEN_destination_ticks_use_ties_to_even )
{
    using namespace std::chrono;
    for ( const auto& [fraction, ticks] : std::vector<std::pair<std::string, long long>> {
                  {"0004999999999999999999999999999999",      0   },
                  {"0005",                                    0   },
                  {"0015",                                    2   },
                  {"0005000001",                              1   },
                  {"000500000000000000000000000000000000001", 1   },
                  {"9995",                                    1000}
    } )
    {
        SCOPED_TRACE( fraction );
        EXPECT_EQ( parse_ticks<milliseconds>( "1970-01-01T00:00:00." + fraction + "Z" ), ticks );
    }
    EXPECT_EQ( parse_ticks<milliseconds>( "1969-12-31T23:59:59.9995Z" ), 0 );
    EXPECT_EQ( parse_ticks<milliseconds>( "1969-12-31T23:59:59.9985Z" ), -2 );
    EXPECT_EQ( parse_ticks<seconds>( "1970-01-01T00:00:00.5Z" ), 0 );
    EXPECT_EQ( parse_ticks<microseconds>( "1970-01-01T00:00:00.0000015Z" ), 2 );
    EXPECT_EQ( parse_ticks<nanoseconds>( "1970-01-01T00:00:00.0000000015Z" ), 2 );
    EXPECT_EQ( ( parse_ticks<duration<long long, std::pico>>( "1970-01-01T00:00:00.0000000000015Z" ) ), 2 );
    EXPECT_EQ( ( parse_ticks<duration<long long, std::ratio<1, 3>>>( "1970-01-01T00:00:00.5Z" ) ), 2 );
    EXPECT_EQ( ( parse_ticks<duration<long long, std::ratio<3, 2>>>( "1970-01-01T00:00:02.25Z" ) ), 2 );
}

TEST_F( JsonCodecs, WHEN_timestamps_have_offsets_THEN_utc_and_rollover_are_preserved )
{
    using namespace std::chrono;
    EXPECT_EQ( parse_ticks<seconds>( "1970-01-01T01:00:00+01:00" ), 0 );
    EXPECT_EQ( parse_ticks<seconds>( "1969-12-31T23:00:00-01:00" ), 0 );
    const auto value = rorm::detail::parse_chrono<sys_time<milliseconds>>( "2026-12-31T23:59:59.9995Z" );
    ASSERT_TRUE( value );
    EXPECT_EQ( rorm::detail::format_chrono( *value ), "2027-01-01T00:00:00.000Z" );
    const auto existing = rorm::detail::parse_chrono<sys_time<milliseconds>>( "2026-09-20T10:30:15.125Z" );
    ASSERT_TRUE( existing );
    EXPECT_EQ( rorm::detail::format_chrono( *existing ), "2026-09-20T10:30:15.125Z" );
}

TEST_F( JsonCodecs, WHEN_timestamp_representations_are_formatted_THEN_ticks_round_trip )
{
    using namespace std::chrono;
    const auto check = []<typename D>( D duration ) {
        const sys_time<D> value( duration );
        const auto        text = rorm::detail::format_chrono( value );
        ASSERT_TRUE( text );
        EXPECT_EQ( rorm::detail::parse_chrono<sys_time<D>>( *text ), value );
    };
    for ( const long long ticks : { -100LL, -2LL, -1LL, 0LL, 1LL, 2LL, 100LL } )
    {
        check( duration<long long, std::ratio<1, 3>>( ticks ) );
        check( duration<long long, std::ratio<7, 13>>( ticks ) );
        check( duration<long long, std::pico>( ticks ) );
    }
    check( nanoseconds( std::numeric_limits<long long>::min() ) );
    check( nanoseconds( std::numeric_limits<long long>::max() ) );
}

TEST_F( JsonCodecs, WHEN_timestamps_are_malformed_THEN_incompatible_value_is_returned )
{
    using namespace std::chrono;
    for ( const auto text : { "2026-02-29T00:00:00Z",
                              "2026-01-01T24:00:00Z",
                              "2026-01-01T00:60:00Z",
                              "2026-01-01T00:00:60Z",
                              "2026-01-01T00:00:00",
                              "2026-01-01T00:00:00.Z",
                              "2026-01-01T00:00:00.1xZ",
                              "2026-01-01T00:00:00Zx",
                              "2026-01-01T00:00:00+24:00" } )
    {
        SCOPED_TRACE( text );
        const auto value = parse_ticks<seconds>( text );
        ASSERT_FALSE( value );
        EXPECT_EQ( value.error(), rorm::make_error_code( rorm::json_error_code::incompatible_value ) );
    }
}

TEST_F( JsonCodecs, WHEN_valid_timestamp_exceeds_representation_THEN_out_of_range_is_returned )
{
    using namespace std::chrono;
    for ( const auto text : { "2262-04-11T23:47:16.8547758075Z", "1677-09-21T00:12:43.1452241914Z" } )
    {
        const auto value = parse_ticks<nanoseconds>( text );
        ASSERT_FALSE( value );
        EXPECT_EQ( value.error(), rorm::make_error_code( rorm::json_error_code::out_of_range ) );
    }
}

class EncodedAccessor
{
public:
    [[= rorm::getter( "data" )]][[= rorm::base64]] const std::vector<unsigned char>& data() const { return data_; }

    [[= rorm::setter( "data" )]][[= rorm::base64]] void data( std::vector<unsigned char> value )
    {
        data_ = std::move( value );
    }

private:
    std::vector<unsigned char> data_;
};

struct ByteStorage
{
    unsigned char value {};
};

std::vector<unsigned char> encode_storage( ByteStorage storage )
{
    return { storage.value };
}

ByteStorage decode_storage( const std::vector<unsigned char>& bytes )
{
    return { bytes.at( 0 ) };
}

struct ConvertedBytes
{
    [[= rorm::base64]][[= rorm::as<std::vector<unsigned char>>( encode_storage, decode_storage )]] ByteStorage data;
};

struct OptionalBytes
{
    [[= rorm::base64]] std::optional<std::vector<unsigned char>> data = std::vector<unsigned char> { 255 };
};

TEST_F( JsonCodecs, WHEN_base64_accessors_round_trip_THEN_logical_bytes_are_preserved )
{
    const auto record = rorm::from_json<EncodedAccessor>( R"({"data":"Zg=="})", {} );
    ASSERT_TRUE( record );
    EXPECT_EQ( record->data(), ( std::vector<unsigned char> { 102 } ) );
    EXPECT_EQ( rorm::to_json( *record ), R"({"data":"Zg=="})" );
}

TEST_F( JsonCodecs, WHEN_base64_converted_values_round_trip_THEN_logical_bytes_are_preserved )
{
    const auto record = rorm::from_json<ConvertedBytes>( R"({"data":"Zg=="})", {} );
    ASSERT_TRUE( record );
    EXPECT_EQ( record->data.value, 102 );
    EXPECT_EQ( rorm::to_json( *record ), R"({"data":"Zg=="})" );
}

TEST_F( JsonCodecs, WHEN_optional_base64_is_missing_or_null_THEN_initial_value_is_cleared )
{
    for ( const auto input : { R"({})", R"({"data":null})" } )
    {
        const auto record = rorm::from_json<OptionalBytes>( input, {} );
        ASSERT_TRUE( record );
        EXPECT_FALSE( record->data );
        EXPECT_EQ( rorm::to_json( *record ), R"({"data":null})" );
    }
    const auto record = rorm::from_json<OptionalBytes>( R"({"data":""})", {} );
    ASSERT_TRUE( record );
    ASSERT_TRUE( record->data );
    EXPECT_TRUE( record->data->empty() );
    EXPECT_EQ( rorm::to_json( *record ), R"({"data":""})" );
}

TEST_F( JsonCodecs, WHEN_timestamp_precision_varies_THEN_standard_and_custom_widths_are_retained )
{
    using namespace std::chrono;
    EXPECT_EQ( rorm::detail::format_chrono( sys_time<seconds>( seconds( 1 ) ) ), "1970-01-01T00:00:01Z" );
    EXPECT_EQ( rorm::detail::format_chrono( sys_time<microseconds>( microseconds( 1 ) ) ), "1970-01-01T00:00:00.000001Z" );
    EXPECT_EQ( rorm::detail::format_chrono( sys_time<nanoseconds>( nanoseconds( 1 ) ) ),
               "1970-01-01T00:00:00.000000001Z" );
    EXPECT_EQ( ( rorm::detail::format_chrono(
                       sys_time<duration<long long, std::ratio<1, 8>>>( duration<long long, std::ratio<1, 8>>( 1 ) ) ) ),
               "1970-01-01T00:00:00.125Z" );
    EXPECT_EQ( parse_ticks<milliseconds>( "1970-01-01T00:00:00Z" ), 0 );
    EXPECT_EQ( parse_ticks<milliseconds>( "1970-01-01T00:00:00.1Z" ), 100 );
    EXPECT_EQ( parse_ticks<milliseconds>( "1970-01-01T00:00:00.123456Z" ), 123 );
}

TEST_F( JsonCodecs, WHEN_calendar_or_unsigned_representation_is_exceeded_THEN_range_error_is_returned )
{
    using namespace std::chrono;
    const auto outside_calendar = rorm::detail::format_chrono( sys_seconds( sys_days( year( -1 ) / January / 1 ) ) );
    ASSERT_FALSE( outside_calendar );
    EXPECT_EQ( outside_calendar.error(), rorm::make_error_code( rorm::json_error_code::out_of_range ) );
    const auto outside = parse_ticks<seconds>( "9999-12-31T23:59:59.5Z" );
    ASSERT_FALSE( outside );
    EXPECT_EQ( outside.error(), rorm::make_error_code( rorm::json_error_code::out_of_range ) );
    const auto negative = parse_ticks<duration<unsigned long long>>( "1969-12-31T23:59:59Z" );
    ASSERT_FALSE( negative );
    EXPECT_EQ( negative.error(), rorm::make_error_code( rorm::json_error_code::out_of_range ) );
}

TEST_F( JsonCodecs, WHEN_offset_leaves_calendar_before_rounding_THEN_range_error_is_returned )
{
    for ( auto text : { "0000-01-01T00:00:00+00:01", "9999-12-31T23:59:59-00:01" } )
    {
        const auto result = parse_ticks<std::chrono::days>( text );
        ASSERT_FALSE( result );
        EXPECT_EQ( result.error(), rorm::make_error_code( rorm::json_error_code::out_of_range ) );
    }
}
} // namespace
