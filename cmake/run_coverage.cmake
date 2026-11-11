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

# Always start with fresh counters, including those written by test discovery.
file(GLOB_RECURSE counters "${BUILD_DIR}/*.gcda")
if(counters)
    file(REMOVE ${counters})
endif()
set(report_directory "${BUILD_DIR}/coverage")
file(REMOVE_RECURSE "${report_directory}")
file(MAKE_DIRECTORY "${report_directory}")

execute_process(
    COMMAND "${CTEST_COMMAND}" --test-dir "${BUILD_DIR}" -C "${CONFIG}" --output-on-failure
    RESULT_VARIABLE test_result
)
# Test failures must not prevent collecting their coverage.
execute_process(
    COMMAND "${GCOVR}" --root "${SOURCE_DIR}"
        --exclude "3rd_party/" --exclude "(.*/)?_deps/"
        --gcov-executable "${GCOV}" --html-details "${report_directory}/index.html"
        --print-summary "${BUILD_DIR}"
    WORKING_DIRECTORY "${SOURCE_DIR}"
    RESULT_VARIABLE coverage_result
)
if(NOT coverage_result STREQUAL "0")
    message(FATAL_ERROR "Coverage collection failed: ${coverage_result}; CTest result: ${test_result}")
endif()
message(STATUS "Coverage report: ${report_directory}/index.html")
if(NOT test_result STREQUAL "0")
    message(FATAL_ERROR "CTest failed (${test_result}); coverage report was generated")
endif()
