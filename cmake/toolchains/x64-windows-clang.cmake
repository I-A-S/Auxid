# Auxid: The Orthodox C++ Platform.
# Copyright (C) 2026 IAS (ias@iasoft.dev)
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

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR AMD64)
set(CMAKE_C_COMPILER clang-cl)
set(CMAKE_CXX_COMPILER clang-cl)
set(CMAKE_RC_COMPILER llvm-rc)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")

set(triple x86_64-pc-windows-msvc)
set(CMAKE_C_COMPILER_TARGET ${triple})
set(CMAKE_CXX_COMPILER_TARGET ${triple})

set(CMAKE_LINKER lld-link)

string(APPEND CMAKE_C_FLAGS   " /arch:AVX2 /clang:-mfma")
string(APPEND CMAKE_CXX_FLAGS " /arch:AVX2 /clang:-mfma")
