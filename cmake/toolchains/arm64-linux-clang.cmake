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

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(_auxid_triple aarch64-linux-gnu)
set(CMAKE_C_COMPILER_TARGET ${_auxid_triple})
set(CMAKE_CXX_COMPILER_TARGET ${_auxid_triple})

set(CMAKE_SYSROOT "/usr/${_auxid_triple}")
set(CMAKE_FIND_ROOT_PATH "${CMAKE_SYSROOT}")
set(CMAKE_LIBRARY_ARCHITECTURE ${_auxid_triple})

string(APPEND CMAKE_C_FLAGS " -march=armv8-a+simd --sysroot=${CMAKE_SYSROOT} --gcc-toolchain=/usr")
string(APPEND CMAKE_CXX_FLAGS " -march=armv8-a+simd --sysroot=${CMAKE_SYSROOT} --gcc-toolchain=/usr")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
