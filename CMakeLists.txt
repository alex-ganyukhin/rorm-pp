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

if (NOT "${COMPILER_DIR}" STREQUAL "")
    set(CMAKE_C_COMPILER "${COMPILER_DIR}/bin/clang")
    set(CMAKE_CXX_COMPILER "${COMPILER_DIR}/bin/clang++")
endif()

MESSAGE(STATUS "CMAKE_C_COMPILER: ${CMAKE_C_COMPILER}")
MESSAGE(STATUS "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")

project(rorm)

set(CMAKE_CXX_STANDARD 26)

add_executable(
    rorm-test
    test/test_reflection.cc
)

target_compile_options(rorm-test PUBLIC -Wall -Wextra -Werror) # fixme -> separate script
target_compile_options(rorm-test PUBLIC -freflection-latest -stdlib=libc++ )
target_link_options(rorm-test PUBLIC -lc++abi -lc++)
