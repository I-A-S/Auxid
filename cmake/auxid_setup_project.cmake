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

macro(auxid_setup_project)

    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "^(Clang|AppleClang|MSVC)$")
        message(FATAL_ERROR "Auxid requires Clang, Clang-CL, or native MSVC. Current compiler detected: ${CMAKE_CXX_COMPILER_ID}")
    endif()

    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

    set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
    set(CMAKE_C_STANDARD 11)
    set(CMAKE_C_STANDARD_REQUIRED ON)
    set(CMAKE_C_EXTENSIONS OFF)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

    if(WIN32)
        add_compile_definitions(NOMINMAX)
    endif()

    if(CMAKE_SYSTEM_PROCESSOR MATCHES "amd64|x86_64|AMD64")
        set(AUXID_ARCH_X64 TRUE CACHE INTERNAL "")
        add_compile_definitions(AUXID_ARCH_X64=1)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64|ARM64")
        set(AUXID_ARCH_ARM64 TRUE CACHE INTERNAL "")
        add_compile_definitions(AUXID_ARCH_ARM64=1)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "wasm32|emscripten")
        set(AUXID_ARCH_WASM TRUE CACHE INTERNAL "")
        add_compile_definitions(AUXID_ARCH_WASM=1)
    else()
        message(WARNING "Auxid: Unrecognized architecture '${CMAKE_SYSTEM_PROCESSOR}'. Proceed with caution.")
    endif()

    if(AUXID_ARCH_X64)
        add_compile_definitions(AUXID_HAS_SSE2=1)
    endif()
    if(AUXID_ARCH_ARM64)
        add_compile_definitions(AUXID_HAS_NEON=1)
    endif()
    if(AUXID_ARCH_WASM)
        add_compile_definitions(AUXID_HAS_WASM_SIMD128=1)
    endif()

    option(AUXID_DISABLE_HASHMAP_SIMD "Force the scalar SwissTable probe path" OFF)
    if(AUXID_DISABLE_HASHMAP_SIMD)
        add_compile_definitions(AUXID_DISABLE_HASHMAP_SIMD=1)
    endif()

    if(EMSCRIPTEN OR AUXID_ARCH_WASM)
        set(AUXID_USE_SYSTEM_MALLOC TRUE CACHE INTERNAL "")
        message(STATUS "Auxid: WASM/Emscripten target detected - using system malloc instead of rpmalloc.")
    endif()

endmacro()