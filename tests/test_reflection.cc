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

// 3pp
#include <gtest/gtest.h>

// STL
#include <iostream>
#include <meta>
#include <string>

using namespace std::string_literals;
struct As
{
    char value[6];
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

    static constexpr auto members = std::define_static_array(
            std::meta::nonstatic_data_members_of( ^^T, std::meta::access_context::current() ) );

    template for ( constexpr auto member : members )
    {
        std::string           name { std::meta::identifier_of( member ) };
        static constexpr auto annotations = std::define_static_array( std::meta::annotations_of( member ) );

        template for ( constexpr auto annotation : annotations )
        {
            constexpr auto column = std::meta::extract<As>( annotation );
            name                  = column.value;
        }

        out << "\"" << name << "\": \"" << obj.[:member:] << "\", ";
    }

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
