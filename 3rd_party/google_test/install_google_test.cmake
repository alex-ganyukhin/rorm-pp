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

include(FetchContent)


set(GTEST_URL "https://github.com/google/googletest/archive/refs/tags/v1.16.0.zip")


message(STATUS "Downloading & configuring google test sources. URL: ${GTEST_URL} ...")

FetchContent_Declare(
        gtest
        # Specify the commit you depend on and update it regularly.
        URL ${GTEST_URL}
)
FetchContent_MakeAvailable(gtest)

message(STATUS "Downloading & configuring google test sources... DONE!")
