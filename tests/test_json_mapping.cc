/**
 * @file    test_json_mapping.cc
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
#include <optional>
#include <rorm/json.h>
#include <string>

namespace rorm_test {
class JsonMappingTest : public testing::Test
{};

struct InitializedOptional
{
    std::optional<int> value { 42 };
};

TEST_F( JsonMappingTest, WHEN_optional_is_missing_THEN_initializer_is_cleared )
{
    auto result = rorm::from_json<InitializedOptional>( "{}", {} );
    ASSERT_TRUE( result );
    EXPECT_FALSE( result->value );
}

struct ReadOnly
{
    [[= rorm::getter( "value" )]] int value() const { return 3; }
};

TEST_F( JsonMappingTest, WHEN_readonly_value_has_another_type_THEN_it_is_skipped )
{
    auto result = rorm::from_json<ReadOnly>( R"({"value":{"ignored":[false]}})", {} );
    ASSERT_TRUE( result );
    EXPECT_EQ( result->value(), 3 );
}

struct OwnedText
{
    std::string text;
};

TEST_F( JsonMappingTest, WHEN_input_storage_is_changed_THEN_decoded_text_is_owned )
{
    std::string input  = R"({"text":"retained"})";
    auto        result = rorm::from_json<OwnedText>( input, {} );
    ASSERT_TRUE( result );
    input.assign( input.size(), 'x' );
    EXPECT_EQ( result->text, "retained" );
}

struct SerializationOnly
{
    explicit SerializationOnly( int v )
        : value( v )
    {}

    int value;
};

TEST_F( JsonMappingTest, WHEN_non_default_constructible_record_is_serialized_THEN_fields_are_encoded )
{
    auto result = rorm::to_json( SerializationOnly { 7 } );
    ASSERT_TRUE( result );
    EXPECT_EQ( nlohmann::json::parse( *result )["value"], 7 );
}
} // namespace rorm_test
