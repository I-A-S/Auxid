cmake_minimum_required(VERSION 3.20)

project(Auxid ASM C CXX)

enable_testing()

include(cmake/auxid_setup_project.cmake)

auxid_setup_project()

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

add_library(auxid_platform INTERFACE)
target_compile_options(auxid_platform INTERFACE ${AUXID_CXX_FLAGS_INTERNAL})
target_link_options(auxid_platform INTERFACE ${AUXID_LINK_FLAGS_INTERNAL})

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

target_link_libraries(libauxid PRIVATE auxid_platform)
