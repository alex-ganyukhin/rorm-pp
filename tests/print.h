/**
 * @file    print.h
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
#include <gtest/gtest.h>
#include <meta>
#include <ostream>

namespace rorm_test {
/**
 * @brief  Opt a test record into reflection-based GoogleTest printing.
 *
 * Specialize to true in rorm_test before any printer use. Partially specialize for template records. See PrintTo for an example.
 */
template<typename T>
inline constexpr bool printable_record = false;

/**
 * @brief  Print an opted-in record's type and direct non-static fields, including private backing state.
 *
 * GoogleTest finds this overload through argument-dependent lookup for records in rorm_test. Member values use GoogleTest printers,
 * including opted-in nested records and containers. Base-class state is not traversed. Unchecked access is confined to test support.
 *
 * @code{.cpp}
 *     namespace rorm_test {
 *     struct Point { int x; int y; };
 *     template<> inline constexpr bool printable_record<Point> = true;
 *     // GoogleTest assertion diagnostics now print named x and y values.
 *     }
 * @endcode
 *
 * @param  value  Record to inspect without copying.
 * @param  out    Non-null output stream supplied by GoogleTest.
 */
template<typename T>
    requires printable_record<T>
void PrintTo( const T& value, std::ostream* out )
{
    *out << std::meta::display_string_of( ^^T ) << "{";
    bool first = true;
    // Inspect private backing state only in opted-in test records.
    static constexpr auto fields = std::define_static_array(
            std::meta::nonstatic_data_members_of( ^^T, std::meta::access_context::unchecked() ) );
    template for ( constexpr auto field : fields )
    {
        if ( !first )
            *out << ", ";
        first = false;
        *out << std::meta::identifier_of( field ) << "=" << testing::PrintToString( value.[:field:] );
    }
    *out << "}";
}
} // namespace rorm_test
