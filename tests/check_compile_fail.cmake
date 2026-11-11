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

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env LC_ALL=C "${COMPILER}"
        -std=c++26 -freflection -Wall -Wextra -Werror -fdiagnostics-plain-output
        "-I${INCLUDE_DIR}" -isystem "${BACKEND_INCLUDE}" -fsyntax-only "${SOURCE}"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error
)
if(NOT result MATCHES "^[0-9]+$" OR result STREQUAL "0")
    message(FATAL_ERROR "Expected compilation failure for ${SOURCE}; result: ${result}\n${output}${error}")
endif()
file(WRITE "${DIAGNOSTICS}" "${output}${error}")
execute_process(
    COMMAND "${FILECHECK}" "${SOURCE}" "--input-file=${DIAGNOSTICS}"
    RESULT_VARIABLE check_result OUTPUT_VARIABLE check_output ERROR_VARIABLE check_error
)
if(NOT check_result STREQUAL "0")
    message(FATAL_ERROR "Diagnostic check failed for ${SOURCE}:\n${check_output}${check_error}")
endif()
