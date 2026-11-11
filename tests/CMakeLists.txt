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

# Test-only JSON oracle; public rorm headers remain backend-independent.
include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    SYSTEM
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
)
FetchContent_MakeAvailable(nlohmann_json)

add_executable(
    ${PROJECT_NAME}
    test_reflection.cc
    test_json.cc
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
    GTest::gtest_main
    rorm
    rorm_warnings
    nlohmann_json::nlohmann_json
)

include(GoogleTest)

gtest_discover_tests(${PROJECT_NAME})

find_program(RORM_FILECHECK NAMES FileCheck FileCheck-19 REQUIRED)
add_test(NAME CompileCheck.Positive
    COMMAND "${CMAKE_CXX_COMPILER}" -std=c++26 -freflection -Wall -Wextra -Werror
        "-I${CMAKE_SOURCE_DIR}/rorm/include" -fsyntax-only
        "${CMAKE_CURRENT_SOURCE_DIR}/compile_positive.cc")
file(GLOB compile_fail_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/compile_fail/*.cc")
foreach(source IN LISTS compile_fail_sources)
    get_filename_component(name "${source}" NAME_WE)
    add_test(NAME JsonMapping.${name}
        COMMAND "${CMAKE_COMMAND}"
            "-DCOMPILER=${CMAKE_CXX_COMPILER}"
            "-DFILECHECK=${RORM_FILECHECK}"
            "-DINCLUDE_DIR=${CMAKE_SOURCE_DIR}/rorm/include"
            "-DSOURCE=${source}"
            "-DDIAGNOSTICS=${CMAKE_CURRENT_BINARY_DIR}/${name}.diagnostics"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/check_compile_fail.cmake")
endforeach()
