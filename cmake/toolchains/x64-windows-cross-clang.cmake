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

# ---------------------------------------------------------------------
# WinSDK directory can be splatted on linux using 'xwin' cargo package
# Please also ensure to place OpenSSL windows binaries in WINSDK_PATH/openssl/x64
# OpenSSL minimum required version is 3.0.0

if(NOT DEFINED ENV{WINSDK_PATH} OR "$ENV{WINSDK_PATH}" STREQUAL "")
    message(FATAL_ERROR "Environment variable WINSDK_PATH is not set. Please set it before running the cross toolchain.")
endif()

message(STATUS "Using Windows SDK at : $ENV{WINSDK_PATH}")

set(OPENSSL_ROOT_DIR "$ENV{WINSDK_PATH}/openssl/x64")
set(OPENSSL_USE_STATIC_LIBS TRUE)

if(NOT EXISTS "${OPENSSL_ROOT_DIR}")
    message(FATAL_ERROR "OpenSSL directory not found: ${OPENSSL_ROOT_DIR}")
elseif(NOT IS_DIRECTORY "${OPENSSL_ROOT_DIR}")
    message(FATAL_ERROR "OpenSSL path exists but is not a directory: ${OPENSSL_ROOT_DIR}")
endif()

string(APPEND CMAKE_C_FLAGS   " /imsvc $ENV{WINSDK_PATH}/crt/include /imsvc $ENV{WINSDK_PATH}/sdk/include/ucrt /imsvc $ENV{WINSDK_PATH}/sdk/include/um /imsvc $ENV{WINSDK_PATH}/sdk/include/shared")
string(APPEND CMAKE_CXX_FLAGS " /imsvc $ENV{WINSDK_PATH}/crt/include /imsvc $ENV{WINSDK_PATH}/sdk/include/ucrt /imsvc $ENV{WINSDK_PATH}/sdk/include/um /imsvc $ENV{WINSDK_PATH}/sdk/include/shared")

set(CRT_LIB_PATH "$ENV{WINSDK_PATH}/crt/lib/x86_64")
set(SDK_LIB_PATH "$ENV{WINSDK_PATH}/sdk/lib")

set(CMAKE_EXE_LINKER_FLAGS 
    "${CMAKE_EXE_LINKER_FLAGS} /libpath:\"${CRT_LIB_PATH}\" /libpath:\"${SDK_LIB_PATH}/ucrt/x86_64\" /libpath:\"${SDK_LIB_PATH}/um/x86_64\""
)

# ---------------------------------------------------------------------
