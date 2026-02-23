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

option(Auxid_BUILD_TESTS "Build unit tests" ${AUXID_IS_TOP_LEVEL})

add_subdirectory(src)

if(Auxid_BUILD_TESTS)
  add_subdirectory(tests)
endif()
