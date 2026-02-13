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
