cmake_minimum_required(VERSION 3.20)

project(Auxid ASM C CXX)

if(NOT CMAKE_CXX_COMPILER_ID MATCHES "^(Clang|AppleClang)$")
    message(FATAL_ERROR "Auxid requires Clang or Clang-CL. Current compiler detected: ${CMAKE_CXX_COMPILER_ID}")
endif()

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

if(MSVC) 
    set(AUXID_CXX_FLAGS_INTERNAL
      "/clang:-nostdlib++"
      "/clang:-ffreestanding"
      "/EHs-c-" 
    )
    set(AUXID_LINK_FLAGS_INTERNAL
      "/NODEFAULTLIB:libcpmt.lib"  
      "/NODEFAULTLIB:msvcprt.lib"   
    )
else()
    set(AUXID_CXX_FLAGS_INTERNAL
      "-nostdlib++"
      "-ffreestanding"
      "-fno-exceptions"
    )
    set(AUXID_LINK_FLAGS_INTERNAL
      "-nostdlib++"
    )
endif()

set(AUXID_CXX_FLAGS "${AUXID_CXX_FLAGS_INTERNAL}" CACHE INTERNAL "Auxid compiler flags")
set(AUXID_LINK_FLAGS "${AUXID_LINK_FLAGS_INTERNAL}" CACHE INTERNAL "Auxid linker flags")

add_library(auxid_compile_options INTERFACE)
target_compile_options(auxid_compile_options INTERFACE ${AUXID_CXX_FLAGS_INTERNAL})
target_link_options(auxid_compile_options INTERFACE ${AUXID_LINK_FLAGS_INTERNAL})

set(SRC_FILES
  "src/cpp/auxid.cpp"

  "src/cpp/vendor/rpmalloc/rpmalloc.c"
  "src/cpp/vendor/tinycthread/tinycthread.c"
)

add_library(libauxid STATIC ${SRC_FILES})

target_include_directories(libauxid PUBLIC
  "include"
)

target_include_directories(libauxid PRIVATE
  "src/hpp"
)

target_link_libraries(libauxid PRIVATE auxid_compile_options)
