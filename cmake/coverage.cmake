# Copyright 2026 Aleksandr Ganiukhin
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

option(ENABLE_COVERAGE "Enable coverage collection" OFF)
if(NOT ENABLE_COVERAGE)
    return()
endif()

find_program(RORM_GCOVR gcovr REQUIRED)
get_filename_component(compiler_directory "${CMAKE_CXX_COMPILER}" DIRECTORY)
find_program(RORM_GCOV gcov HINTS "${compiler_directory}" REQUIRED)

add_compile_options(--coverage -O0 -g)
add_link_options(--coverage)

add_custom_target(coverage
    COMMAND "${CMAKE_COMMAND}"
        "-DBUILD_DIR=${CMAKE_BINARY_DIR}"
        "-DSOURCE_DIR=${CMAKE_SOURCE_DIR}"
        "-DCTEST_COMMAND=${CMAKE_CTEST_COMMAND}"
        "-DCONFIG=$<CONFIG>"
        "-DGCOVR=${RORM_GCOVR}"
        "-DGCOV=${RORM_GCOV}"
        -P "${CMAKE_CURRENT_LIST_DIR}/run_coverage.cmake"
    DEPENDS rorm_test
    USES_TERMINAL
    VERBATIM
)
