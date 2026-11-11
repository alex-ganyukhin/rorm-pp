/**
 * @file    matchers.h
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
#pragma once
#include <gmock/gmock.h>
#include <nlohmann/json.hpp>
#include <type_traits>

/**
 * @brief  Match the value branch of a non-void std::expected with an inner matcher.
 *
 * Reports the actual value or unexpected error and forwards the inner matcher's explanation. Contents are inspected without copying.
 *
 * @code{.cpp}
 *     EXPECT_THAT( rorm::to_json( record ), ExpectedValue( JsonEq( expected_json ) ) );
 * @endcode
 */
MATCHER_P( ExpectedValue,
           inner,
           "has a value matching " + testing::DescribeMatcher<typename std::remove_cvref_t<arg_type>::value_type>( inner ) )
{
    if ( !arg.has_value() )
    {
        *result_listener << "has error " << testing::PrintToString( arg.error() );
        return false;
    }
    *result_listener << "has value " << testing::PrintToString( *arg ) << ", ";
    return testing::ExplainMatchResult( inner, *arg, result_listener );
}

/**
 * @brief  Match the error branch of a non-void std::expected with an inner matcher.
 *
 * Reports the actual error or unexpected value and forwards the inner matcher's explanation. Contents are inspected without copying.
 *
 * @code{.cpp}
 *     EXPECT_THAT( result, ExpectedError( testing::Eq( expected_error ) ) );
 * @endcode
 */
MATCHER_P( ExpectedError,
           inner,
           "has an error matching " +
                   testing::DescribeMatcher<typename std::remove_cvref_t<arg_type>::error_type>( inner ) )
{
    if ( arg.has_value() )
    {
        *result_listener << "has value " << testing::PrintToString( *arg );
        return false;
    }
    *result_listener << "has error " << testing::PrintToString( arg.error() ) << ", ";
    return testing::ExplainMatchResult( inner, arg.error(), result_listener );
}

/**
 * @brief  Compare JSON text by parsed value, ignoring whitespace and object-member order.
 *
 * Array order remains significant. Invalid actual or expected JSON fails the match with an explanation; unequal values are printed.
 * Input types must be accepted by nlohmann::json::parse. Borrowed expected text must remain valid until matching finishes.
 *
 * @code{.cpp}
 *     EXPECT_THAT( R"({"a":1,"b":2})", JsonEq( R"({"b":2,"a":1})" ) );
 * @endcode
 */
MATCHER_P( JsonEq, expected, "is JSON equal to " + testing::PrintToString( expected ) )
{
    const auto actual_json   = nlohmann::json::parse( arg, nullptr, false );
    const auto expected_json = nlohmann::json::parse( expected, nullptr, false );
    if ( actual_json.is_discarded() || expected_json.is_discarded() )
    {
        *result_listener << "invalid " << ( actual_json.is_discarded() ? "actual" : "expected" )
                         << " JSON; actual: " << testing::PrintToString( arg )
                         << "; expected: " << testing::PrintToString( expected );
        return false;
    }
    if ( actual_json == expected_json )
        return true;
    *result_listener << "actual JSON: " << actual_json.dump() << "; expected JSON: " << expected_json.dump();
    return false;
}
