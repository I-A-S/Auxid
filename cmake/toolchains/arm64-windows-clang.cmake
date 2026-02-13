set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_C_COMPILER clang-cl)
set(CMAKE_CXX_COMPILER clang-cl)
set(CMAKE_RC_COMPILER llvm-rc)

set(CMAKE_LINKER lld-link)

set(triple arm64-pc-windows-msvc)
set(CMAKE_C_COMPILER_TARGET   ${triple})
set(CMAKE_CXX_COMPILER_TARGET ${triple})

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

string(APPEND CMAKE_C_FLAGS   " /clang:--target=arm64-pc-windows-msvc")
string(APPEND CMAKE_CXX_FLAGS " /clang:--target=arm64-pc-windows-msvc")
