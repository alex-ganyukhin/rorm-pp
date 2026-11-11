/**
 * @file    compile_positive.cc
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
#include <concepts>
#include <rorm/rorm.h>
#include <string>
#include <type_traits>
#include <vector>

static_assert( std::same_as<decltype( rorm::name( "value" ) ), rorm::name_annotation> );
static_assert( std::same_as<decltype( rorm::getter( "value" ) ), rorm::getter_annotation> );
static_assert( std::same_as<decltype( rorm::setter( "value" ) ), rorm::setter_annotation> );
static_assert( std::same_as<decltype( rorm::table( "value" ) ), rorm::table_annotation> );
static_assert( std::same_as<std::remove_cv_t<decltype( rorm::ignore )>, rorm::ignore_annotation> );
static_assert( std::same_as<std::remove_cv_t<decltype( rorm::primary_key )>, rorm::primary_key_annotation> );
static_assert( std::same_as<std::remove_cv_t<decltype( rorm::generated )>, rorm::generated_annotation> );
static_assert( std::same_as<std::remove_cv_t<decltype( rorm::base64 )>, rorm::base64_annotation> );
constexpr auto identity = []( int value ) { return value; };
static_assert( std::same_as<decltype( rorm::as<int>( identity ) ),
                            rorm::conversion_annotation<int, std::remove_cv_t<decltype( identity )>>> );
static_assert( std::same_as<
               decltype( rorm::as<int>( identity, identity ) ),
               rorm::conversion_annotation<int, std::remove_cv_t<decltype( identity )>, std::remove_cv_t<decltype( identity )>>> );

struct Positive
{
    [[= rorm::name( "number" )]] int number {};
    [[= rorm::ignore]] int           excluded {};

    [[= rorm::getter( "accessor" )]] int read() const { return 1; }

    [[= rorm::setter( "accessor" )]] void write( int ) {}

    [[= rorm::base64]] std::vector<unsigned char>                                                   bytes;
    [[= rorm::as<int>( []( int value ) { return value; }, []( int value ) { return value; } )]] int converted {};
};

int main()
{
    auto encoded = rorm::to_json( Positive {} );
    return encoded.has_value() ? 0 : 1;
}
