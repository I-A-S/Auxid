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

set(AUXID_BENCH_NOEXC_SRC
    "cpp/main.cpp"

    "cpp/containers/hash_map_shared.hpp"
    "cpp/containers/hash_map_vs_unordered_map.cpp"
    "cpp/core/result_vs_exceptions_shared.hpp"
    "cpp/core/result_vs_exceptions.cpp"
)

execute_process(
    COMMAND git rev-parse HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE AUXID_BENCH_GIT_COMMIT
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)
if(NOT AUXID_BENCH_GIT_COMMIT)
    set(AUXID_BENCH_GIT_COMMIT "unknown")
endif()
string(SUBSTRING "${AUXID_BENCH_GIT_COMMIT}" 0 7 AUXID_BENCH_COMMIT_SHORT)

add_executable(AuxidBench ${AUXID_BENCH_NOEXC_SRC})
set_target_properties(AuxidBench PROPERTIES CXX_SCAN_FOR_MODULES ON)
target_compile_definitions(AuxidBench PRIVATE
    AUXID_BENCH_COMMIT="${AUXID_BENCH_COMMIT_SHORT}"
)

add_library(auxid_bench_throw_baseline OBJECT
    "cpp/core/exception_baseline.cpp"
)
set_target_properties(auxid_bench_throw_baseline PROPERTIES
    CXX_SCAN_FOR_MODULES OFF
)
target_link_libraries(auxid_bench_throw_baseline PRIVATE benchmark::benchmark)

if(MSVC)
    target_compile_options(auxid_bench_throw_baseline PRIVATE /EHsc)
else()
    target_compile_options(auxid_bench_throw_baseline PRIVATE -fexceptions)
endif()

target_link_libraries(AuxidBench
    PRIVATE
        libauxid
        benchmark::benchmark
        auxid_bench_throw_baseline
)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND WIN32)
    target_link_libraries(AuxidBench PRIVATE stdc++exp)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(AuxidBench PRIVATE
        -Wno-deprecated-declarations
    )
endif()

get_target_property(_auxid_bench_compile_options AuxidBench COMPILE_OPTIONS)
if(_auxid_bench_compile_options)
    target_compile_options(auxid_bench_throw_baseline PRIVATE ${_auxid_bench_compile_options})
endif()

include(${CMAKE_SOURCE_DIR}/cmake/auxid_msvc_pgo.cmake)
auxid_msvc_pgo_target(libauxid)
auxid_msvc_pgo_target(AuxidBench)
auxid_msvc_pgo_target(auxid_bench_throw_baseline)

if(AUXID_MSVC_PGO_PHASE STREQUAL "INSTRUMENT")
    add_custom_target(auxid_bench_pgo_train
        COMMAND "$<TARGET_FILE:AuxidBench>"
                --benchmark_min_time=0.05s
        DEPENDS AuxidBench
        VERBATIM
        COMMENT "Collecting MSVC PGO training data (runs full AuxidBench suite)"
    )
endif()
