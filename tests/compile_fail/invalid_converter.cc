/**
 * @file    invalid_converter.cc
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
#include <rorm/rorm.h>
#include <string>
#include <vector>

// CHECK: error: {{.*}}convert
struct Record
{
    [[= rorm::as<std::string>( []( int ) { return 1; }, []( std::string ) { return 1; } )]] int value {};
};

int main()
{
    auto encoded = rorm::to_json( Record {} );
    auto decoded = rorm::from_json<Record>( "{}", {} );
    return encoded.has_value() && decoded.has_value() ? 0 : 1;
}
