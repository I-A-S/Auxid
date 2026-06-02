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

set(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_SYSTEM_PROCESSOR arm64)

set(_auxid_llvm_root "/opt/homebrew/opt/llvm")
if(NOT EXISTS "${_auxid_llvm_root}/bin/clang++")
    message(FATAL_ERROR
        "Auxid on macOS requires Homebrew LLVM Clang (brew install llvm). "
        "Expected compiler at: ${_auxid_llvm_root}/bin/clang++")
endif()
