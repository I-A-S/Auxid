# Auxid: The Rigid C++ Platform.
#
# Copyright (C) 2026 I-A-S (ias@iasoft.dev)
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

include_guard(GLOBAL)

include(FetchContent)

set(BENCHMARK_ENABLE_TESTING      OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_INSTALL      OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS  OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_WERROR       OFF CACHE BOOL "" FORCE)
set(BENCHMARK_INSTALL_DOCS        OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_EXCEPTIONS   ON  CACHE BOOL "" FORCE)
set(BENCHMARK_DOWNLOAD_DEPENDENCIES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    google_benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG        v1.9.4
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(google_benchmark)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_ID MATCHES "Clang")
    foreach(_bm_target benchmark benchmark_main)
        if(TARGET ${_bm_target})
            target_compile_options(${_bm_target} PRIVATE
                -Wno-unknown-warning-option
                -Wno-c2y-extensions
            )
        endif()
    endforeach()
endif()
