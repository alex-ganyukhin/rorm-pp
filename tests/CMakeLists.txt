# Copyright 2025 Aleksandr Ganiukhin
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.25)

enable_testing()

project(rorm_test)

add_executable(
    ${PROJECT_NAME}
    test_reflection.cc
)

target_link_libraries(
    ${PROJECT_NAME}
    GTest::gtest_main
)

include(GoogleTest)

# fixme
# - put in common, separate script
# - consider better options
target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Wextra -Werror -Wno-unknown-attributes -fno-access-contexts)
target_compile_options(${PROJECT_NAME} PUBLIC -freflection-latest -stdlib=libc++ )
target_link_options(${PROJECT_NAME} PUBLIC -lc++abi -lc++ -v)
