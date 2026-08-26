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

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR AMD64)

string(APPEND CMAKE_C_FLAGS " -mavx2 -mfma")
string(APPEND CMAKE_CXX_FLAGS " -mavx2 -mfma")

# Runtime libraries MUST stay dynamic on mingw-gcc. Static libgcc links
# emutls into the executable; emutls's first-touch allocates via calloc,
# which binds to rpmalloc's global override, whose thread-heap variable is
# itself emutls under mingw (GCC has no native PE TLS) -> infinite mutual
# recursion (stack-overflow SIGSEGV). Static winpthread livelocks the same
# way through pthread_once. Dynamic libgcc/winpthread resolve their calloc
# against ucrt inside the DLL, never entering rpmalloc.
#
# PATH hermeticity (Git-for-Windows/Strawberry/msys2 all ship divergent
# runtime DLLs; a mismatch dies at load with STATUS_ENTRYPOINT_NOT_FOUND) is
# achieved by copying the toolchain's libstdc++-6/libgcc_s_seh-1/
# libwinpthread-1 DLLs beside every executable instead — see
# tests/CMakeLists.txt.
