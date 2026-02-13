cmake_minimum_required(VERSION 3.20)

project(Auxid ASM C CXX)

enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(AUXID_IS_TOP_LEVEL ON)
else()
    set(AUXID_IS_TOP_LEVEL OFF)
endif()

set(AUXID_ROOT "${CMAKE_CURRENT_LIST_DIR}" CACHE INTERNAL "")

option(Auxid_BUILD_TOOLCHAIN "Build Toolchain" ${AUXID_IS_TOP_LEVEL})
option(Auxid_BUILD_IDE_EXTENSIONS "Build IDE Extensions" ${AUXID_IS_TOP_LEVEL})

add_compile_options(
  "-nostdlib++"
  "-ffreestanding"
)

add_subdirectory(libauxid)

if(Auxid_BUILD_TOOLCHAIN)
  add_subdirectory(toolchain)
endif()

if(Auxid_BUILD_IDE_EXTENSIONS)
  add_subdirectory(ide)
endif()
