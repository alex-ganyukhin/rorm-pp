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



set(CMAKE_CXX_EXTENSIONS OFF)


if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(FATAL_ERROR "rorm-pp requires GCC 16.1 or newer")
endif()

if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 16.1)
    message(FATAL_ERROR "rorm-pp requires GCC 16.1 or newer; found ${CMAKE_CXX_COMPILER_VERSION}")
endif()


add_library(rorm_compiler_options INTERFACE)
target_compile_features(rorm_compiler_options INTERFACE cxx_std_26)
target_compile_options(rorm_compiler_options INTERFACE -freflection)


add_library(rorm_warnings INTERFACE)
target_compile_options(
    rorm_warnings
    INTERFACE
    -Wall
    -Wextra
    -Werror
)
