/**
 * Copyright 2025 Aleksandr Ganiukhin
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
 *
 * @file     test_reflection.cc
 * @date     2025-03-02
 *
 * @author   Alexander Ganyukhin (aganyukhin@outlook.com)
 *
 * @brief
 */

#include "rorm/expand.h"

// 3pp
#include <gtest/gtest.h>

// STL
#include <experimental/meta>
#include <iostream>
#include <ranges>
#include <string>
#include <string_view>

using namespace std::string_literals;
using namespace std::string_view_literals;

struct As
{
    char const* value;
};

struct S
{
    [[= As { "proof" }]] std::string s;

    int i;
};

template<typename T>
void serialize_to_ostream( std::ostream& out, T const& obj )
{
    out << "{";

    [:expand( std::meta::nonstatic_data_members_of( ^^T ) ):] >> [&]<auto e> {
        std::string_view name = std::meta::identifier_of( e );
        [:expand( std::meta::annotations_of( e ) ):] >> [&]<std::meta::info ann> {
            name = extract<[:std::meta::type_of( ann ):]>( ann ).value;
        };

        out << "\"" << name << "\": \"" << obj.[:e:] << "\", ";
    };

    out << "}";
}

TEST( TestReflection, Test1 )
{
    std::string expected = { R"({"proof": "hello", "i": "220", })" };

    std::stringstream sstream;

    S s { "hello", 220 };
    serialize_to_ostream( sstream, s );

    ASSERT_EQ( expected, sstream.str() );
}
