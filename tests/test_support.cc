/**
 * @file    test_support.cc
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
#include <expected>
#include <memory>
#include <optional>
#include <sstream>
#include <vector>

namespace rorm_test {
struct Printable
{
    int              number;
    std::vector<int> items;
    auto             operator<=>( const Printable& ) const = default;
};

template<>
inline constexpr bool printable_record<Printable> = true;

template<typename T>
struct Wrapped
{
    T    inner;
    auto operator<=>( const Wrapped& ) const = default;
};

template<typename T>
inline constexpr bool printable_record<Wrapped<T>> = true;

class PrivateRecord
{
    int hidden_ = 17;

public:
    auto operator<=>( const PrivateRecord& ) const = default;
};

template<>
inline constexpr bool printable_record<PrivateRecord> = true;

class SupportTest : public testing::Test
{};

TEST_F( SupportTest, WHEN_matching_expected_branches_THEN_contents_are_compared )
{
    EXPECT_THAT( ( std::expected<int, int> { 42 } ), ExpectedValue( testing::Eq( 42 ) ) );
    EXPECT_THAT( ( std::expected<int, int> { std::unexpected( 7 ) } ), ExpectedError( testing::Eq( 7 ) ) );
    EXPECT_THAT( ( std::expected<std::unique_ptr<int>, int> { std::make_unique<int>( 42 ) } ),
                 ExpectedValue( testing::Pointee( 42 ) ) );
    EXPECT_THAT( ( std::expected<int, std::unique_ptr<int>> { std::unexpected( std::make_unique<int>( 7 ) ) } ),
                 ExpectedError( testing::Pointee( 7 ) ) );
}

TEST_F( SupportTest, WHEN_expected_branch_or_contents_differ_THEN_match_fails_with_diagnostics )
{
    const std::expected<int, int>      value { 42 }, error { std::unexpected( 7 ) };
    testing::StringMatchResultListener listener;
    EXPECT_FALSE( testing::ExplainMatchResult( ExpectedValue( testing::Eq( 1 ) ), error, &listener ) );
    EXPECT_THAT( listener.str(), testing::HasSubstr( "7" ) );
    listener.Clear();
    EXPECT_FALSE( testing::ExplainMatchResult( ExpectedError( testing::Eq( 1 ) ), value, &listener ) );
    EXPECT_THAT( listener.str(), testing::HasSubstr( "42" ) );
    EXPECT_THAT( value, testing::Not( ExpectedValue( testing::Eq( 0 ) ) ) );
    EXPECT_THAT( error, testing::Not( ExpectedError( testing::Eq( 0 ) ) ) );
}

TEST_F( SupportTest, WHEN_comparing_json_THEN_format_and_member_order_are_ignored )
{
    EXPECT_THAT( R"({"a":1,"b":2})", JsonEq( R"({ "b": 2, "a": 1 })" ) );
    EXPECT_THAT( "1", testing::Not( JsonEq( "2" ) ) );
    EXPECT_THAT( "{", testing::Not( JsonEq( "{}" ) ) );
    EXPECT_THAT( "{}", testing::Not( JsonEq( "{" ) ) );
}

TEST_F( SupportTest, WHEN_printing_opted_in_record_THEN_fields_and_values_are_visible )
{
    const Printable value {
            42,
            { 1, 2 }
    };
    EXPECT_THAT( testing::PrintToString( value ),
                 testing::AllOf( testing::HasSubstr( "number=42" ), testing::HasSubstr( "items=" ) ) );
    static_assert( !printable_record<std::vector<int>> );
}

TEST_F( SupportTest, WHEN_printing_nested_and_private_records_THEN_named_state_is_visible )
{
    const Wrapped<Printable> nested {
            { 42, { 1, 2 } }
    };
    EXPECT_THAT(
            testing::PrintToString( nested ),
            testing::AllOf( testing::HasSubstr( "inner=rorm_test::Printable{" ), testing::HasSubstr( "number=42" ) ) );
    EXPECT_THAT( testing::PrintToString( PrivateRecord {} ), testing::HasSubstr( "hidden_=17" ) );
    const testing::Matcher<const Printable&> matcher = testing::Eq( Printable {
            42,
            { 1, 2 }
    } );
    std::ostringstream                       description;
    matcher.DescribeTo( &description );
    EXPECT_THAT( description.str(), testing::HasSubstr( "number=42" ) );
}

TEST_F( SupportTest, WHEN_json_does_not_match_THEN_explanation_identifies_input )
{
    testing::StringMatchResultListener listener;
    EXPECT_FALSE( testing::ExplainMatchResult( JsonEq( "2" ), "1", &listener ) );
    EXPECT_THAT( listener.str(),
                 testing::AllOf( testing::HasSubstr( "actual JSON: 1" ), testing::HasSubstr( "expected JSON: 2" ) ) );
    listener.Clear();
    EXPECT_FALSE( testing::ExplainMatchResult( JsonEq( "{}" ), "{", &listener ) );
    EXPECT_THAT( listener.str(), testing::HasSubstr( "invalid actual" ) );
    listener.Clear();
    EXPECT_FALSE( testing::ExplainMatchResult( JsonEq( "{" ), "{}", &listener ) );
    EXPECT_THAT( listener.str(), testing::HasSubstr( "invalid expected" ) );
}

TEST_F( SupportTest, WHEN_inner_matcher_explains_failure_THEN_expected_matchers_forward_it )
{
    const std::expected<std::vector<int>, std::vector<int>> value {
            { 1, 2 }
    },
            error { std::unexpected( std::vector<int> { 1, 2 } ) };
    testing::StringMatchResultListener listener;
    EXPECT_FALSE( testing::ExplainMatchResult( ExpectedValue( testing::SizeIs( 3 ) ), value, &listener ) );
    EXPECT_THAT( listener.str(), testing::HasSubstr( "size 2" ) );
    listener.Clear();
    EXPECT_FALSE( testing::ExplainMatchResult( ExpectedError( testing::SizeIs( 3 ) ), error, &listener ) );
    EXPECT_THAT( listener.str(), testing::HasSubstr( "size 2" ) );
}
} // namespace rorm_test
