/**
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
 *
 * @file     reflection-smoke.cc
 * @date     2026-07-13
 *
 * @author   Alexander Ganyukhin (aganyukhin@outlook.com)
 *
 * @brief    Standalone compile check for the GCC C++26 reflection toolchain.
 */

#include <meta>
#include <string_view>

#ifndef __cpp_impl_reflection
#error "C++26 reflection is required"
#elif __cpp_impl_reflection < 202603L
#error "C++26 reflection support is too old"
#endif

#ifndef __cpp_expansion_statements
#error "C++26 expansion statements are required"
#elif __cpp_expansion_statements < 202506L
#error "C++26 expansion statement support is too old"
#endif

struct ColumnName
{
    char value[6];
};

struct Model
{
    [[= ColumnName { "proof" }]] int first;
    int                              second;
};

int main()
{
    Model model { 7, 8 };
    int   sum = 0;

    static constexpr auto members = std::define_static_array(
            std::meta::nonstatic_data_members_of( ^^Model, std::meta::access_context::current() ) );
    static_assert( members.size() == 2 );

    template for ( constexpr auto member : members )
    {
        sum                              += model.[:member:];
        static constexpr auto annotations = std::define_static_array( std::meta::annotations_of( member ) );

        template for ( constexpr auto annotation : annotations )
        {
            constexpr auto column = std::meta::extract<ColumnName>( annotation );
            static_assert( std::string_view( column.value ) == "proof" );
        }
    }

    return sum == 15 ? 0 : 1;
}
