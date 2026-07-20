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

cmake_minimum_required(VERSION 3.28)

project(Auxid VERSION 0.1.0 LANGUAGES C CXX)

enable_testing()

set(AUXID_ROOT "${CMAKE_CURRENT_LIST_DIR}" CACHE INTERNAL "")

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(AUXID_IS_TOP_LEVEL ON)
else()
    set(AUXID_IS_TOP_LEVEL OFF)
endif()

include(cmake/auxid_setup_project.cmake)

auxid_setup_project()

option(Auxid_BUILD_TESTS "Build unit tests" ${AUXID_IS_TOP_LEVEL})
option(Auxid_BUILD_BENCHMARKS "Build microbenchmarks (requires Google Benchmark via FetchContent)" OFF)
option(Auxid_BUILD_SHARED "Build libauxid as a shared library" OFF)

if(Auxid_BUILD_SHARED AND EMSCRIPTEN)
    message(FATAL_ERROR
        "Auxid_BUILD_SHARED is not supported for Emscripten/WASM targets.")
endif()

if(EMSCRIPTEN)
    add_compile_options(
        -msimd128
        -pthread
    )

    add_link_options(
        -pthread
        -sPTHREAD_POOL_SIZE=4
        -sPROXY_TO_PTHREAD
        -sEXIT_RUNTIME=1
        -sALLOW_MEMORY_GROWTH=1
        -sINITIAL_MEMORY=64MB
    )
endif()

add_subdirectory(src)

if(AUXID_IS_TOP_LEVEL)
    include(cmake/auxid_install.cmake)
endif()

if(Auxid_BUILD_TESTS)
  add_subdirectory(tests)

  add_test(
    NAME RigidCppBanList
    COMMAND ${CMAKE_COMMAND}
            -DAUXID_SRC_DIR=${AUXID_ROOT}/src
            -P ${AUXID_ROOT}/cmake/auxid_lint_ban_list.cmake
  )
endif()

if(Auxid_BUILD_BENCHMARKS)
  include(cmake/auxid_fetch_google_benchmark.cmake)
  add_subdirectory(benchmarks)
endif()
