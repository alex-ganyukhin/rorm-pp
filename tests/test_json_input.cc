/**
 * @file    test_json_input.cc
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
#include <cstdint>
#include <gtest/gtest.h>
#include <limits>
#include <rorm/json.h>
#include <string>

namespace {
class JsonInput : public ::testing::Test
{};

TEST_F( JsonInput, WHEN_nested_duplicate_keys_are_read_THEN_duplicate_member_is_returned )
{
    for ( const auto input :
          { R"({"ignored":{"x":1,"x":2}})", R"({"ignored":[{"x":1,"x":2}]})", R"({"name":1,"\u006eame":2})" } )
    {
        SCOPED_TRACE( input );
        const auto result = rorm::detail::parse_json( input );
        ASSERT_FALSE( result );
        EXPECT_EQ( result.error(), rorm::make_error_code( rorm::json_error_code::duplicate_member ) );
    }
}

TEST_F( JsonInput, WHEN_invalid_encoding_or_syntax_is_read_THEN_malformed_input_is_returned )
{
    for ( const auto& input : { std::string( R"({"ignored":[1,]})" ),
                                std::string( R"({"ignored":"\uD800"})" ),
                                std::string( "{\"ignored\":\"" ) + char( 0xff ) + "\"}",
                                std::string( "{} trailing" ),
                                std::string( "{}\0{}", 5 ) } )
    {
        SCOPED_TRACE( input );
        const auto result = rorm::detail::parse_json( input );
        ASSERT_FALSE( result );
        EXPECT_EQ( result.error(), rorm::make_error_code( rorm::json_error_code::malformed_input ) );
    }
}

TEST_F( JsonInput, WHEN_numeric_tokens_exceed_backend_domain_THEN_out_of_range_is_returned )
{
    for ( const auto input :
          { "18446744073709551616", "-9223372036854775809", "1e400", "1e-400", "-0.00000000000000000000000001e-400" } )
    {
        SCOPED_TRACE( input );
        const auto result = rorm::detail::parse_json( input );
        ASSERT_FALSE( result );
        EXPECT_EQ( result.error(), rorm::make_error_code( rorm::json_error_code::out_of_range ) );
    }
}

TEST_F( JsonInput, WHEN_valid_boundary_tokens_and_embedded_nul_are_read_THEN_values_are_preserved )
{
    const auto result = rorm::detail::parse_json( R"({"min":-9223372036854775808,"max":18446744073709551615,)"
                                                  R"("zero":0e-400,"text":"a\u0000b","nested":[{"x":1},{"x":2}]})" );
    ASSERT_TRUE( result );
    EXPECT_EQ( ( *result )["min"].get<std::int64_t>(), std::numeric_limits<std::int64_t>::min() );
    EXPECT_EQ( ( *result )["max"].get<std::uint64_t>(), std::numeric_limits<std::uint64_t>::max() );
    EXPECT_EQ( ( *result )["text"].get<std::string>(), std::string( "a\0b", 3 ) );
    EXPECT_EQ( ( *result )["zero"], 0.0 );
}
} // namespace
