/**
 * @file    test_json_values.cc
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
#include <rorm/json.h>

namespace rorm_test {
class JsonValuesTest : public testing::Test
{};

template<typename T>
struct ValueRecord
{
    T value;
};
enum class ValueStatus
{
    pending[[= rorm::name( "waiting" )]],
    active
};

TEST_F( JsonValuesTest, WHEN_float_rounds_THEN_nearest_representable_value_is_returned )
{
    auto result = rorm::from_json<ValueRecord<float>>( R"({"value":1.00000001})", {} );
    ASSERT_TRUE( result );
    EXPECT_EQ( result->value, 1.0f );
}

TEST_F( JsonValuesTest, WHEN_float_overflows_or_underflows_THEN_range_error_is_returned )
{
    for ( auto text : { R"({"value":1e39})", R"({"value":1e-50})" } )
    {
        auto result = rorm::from_json<ValueRecord<float>>( text, {} );
        ASSERT_FALSE( result );
        EXPECT_EQ( result.error(), rorm::make_error_code( rorm::json_error_code::out_of_range ) );
    }
}

TEST_F( JsonValuesTest, WHEN_subnormal_is_read_THEN_it_is_preserved )
{
    auto result = rorm::from_json<ValueRecord<float>>( R"({"value":1.401298464324817e-45})", {} );
    ASSERT_TRUE( result );
    EXPECT_EQ( result->value, std::numeric_limits<float>::denorm_min() );
}

TEST_F( JsonValuesTest, WHEN_stored_text_is_invalid_utf8_THEN_encoding_fails )
{
    auto result = rorm::to_json( ValueRecord<std::string> { std::string( 1, '\xff' ) } );
    ASSERT_FALSE( result );
    EXPECT_EQ( result.error(), rorm::make_error_code( rorm::json_error_code::incompatible_value ) );
}

TEST_F( JsonValuesTest, WHEN_scalar_category_differs_THEN_no_coercion_occurs )
{
    EXPECT_FALSE( rorm::from_json<ValueRecord<bool>>( R"({"value":1})", {} ) );
    EXPECT_FALSE( rorm::from_json<ValueRecord<std::string>>( R"({"value":false})", {} ) );
    EXPECT_FALSE( rorm::from_json<ValueRecord<float>>( R"({"value":"1"})", {} ) );
    EXPECT_FALSE( rorm::from_json<ValueRecord<float>>( R"({"value":true})", {} ) );
}

TEST_F( JsonValuesTest, WHEN_renamed_enumerator_is_read_THEN_value_matches )
{
    auto result = rorm::from_json<ValueRecord<ValueStatus>>( R"({"value":"waiting"})", {} );
    ASSERT_TRUE( result );
    EXPECT_EQ( result->value, ValueStatus::pending );
}

TEST_F( JsonValuesTest, WHEN_ordinary_enumerator_is_encoded_THEN_name_matches )
{
    auto result = rorm::to_json( ValueRecord<ValueStatus> { ValueStatus::active } );
    ASSERT_TRUE( result );
    EXPECT_EQ( *result, R"({"value":"active"})" );
}

TEST_F( JsonValuesTest, WHEN_unmapped_enum_is_encoded_THEN_encoding_fails )
{
    EXPECT_FALSE( rorm::to_json( ValueRecord<ValueStatus> { static_cast<ValueStatus>( 42 ) } ) );
}

TEST_F( JsonValuesTest, WHEN_empty_vector_is_read_THEN_empty_sequence_is_returned )
{
    auto result = rorm::from_json<ValueRecord<std::vector<int>>>( R"({"value":[]})", {} );
    ASSERT_TRUE( result );
    EXPECT_TRUE( result->value.empty() );
}

TEST_F( JsonValuesTest, WHEN_fixed_sequence_is_too_long_THEN_decoding_fails )
{
    EXPECT_FALSE( ( rorm::from_json<ValueRecord<std::array<int, 1>>>( R"({"value":[1,2]})", {} ) ) );
}

TEST_F( JsonValuesTest, WHEN_sequence_kind_or_element_differs_THEN_decoding_fails )
{
    EXPECT_FALSE( rorm::from_json<ValueRecord<std::vector<int>>>( R"({"value":{}})", {} ) );
    EXPECT_FALSE( rorm::from_json<ValueRecord<std::vector<int>>>( R"({"value":[1,"bad"]})", {} ) );
}

TEST_F( JsonValuesTest, WHEN_byte_is_out_of_range_THEN_range_error_is_returned )
{
    for ( auto text : { R"({"value":[-1]})", R"({"value":[256]})" } )
    {
        auto result = rorm::from_json<ValueRecord<std::vector<std::byte>>>( text, {} );
        ASSERT_FALSE( result );
        EXPECT_EQ( result.error(), rorm::make_error_code( rorm::json_error_code::out_of_range ) );
    }
}

TEST_F( JsonValuesTest, WHEN_later_nested_element_fails_THEN_no_partial_record_is_returned )
{
    auto result = rorm::from_json<ValueRecord<std::vector<ValueRecord<int>>>>( R"({"value":[{"value":1},{}]})", {} );
    ASSERT_FALSE( result );
    EXPECT_EQ( result.error(), rorm::make_error_code( rorm::json_error_code::missing_property ) );
}
} // namespace rorm_test
