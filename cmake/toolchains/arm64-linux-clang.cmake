set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(triple aarch64-linux-gnu)
set(CMAKE_C_COMPILER_TARGET   ${triple})
set(CMAKE_CXX_COMPILER_TARGET ${triple})

set(CMAKE_SYSROOT /usr/aarch64-linux-gnu/sys-root)
set(CMAKE_LIBRARY_ARCHITECTURE aarch64-linux-gnu)

string(APPEND CMAKE_C_FLAGS " -march=armv8-a+simd")
string(APPEND CMAKE_CXX_FLAGS " -march=armv8-a+simd")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

link_directories(${CMAKE_SYSROOT}/usr/lib64)
link_directories(${CMAKE_SYSROOT}/lib64)

