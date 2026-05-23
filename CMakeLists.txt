cmake_minimum_required(VERSION 3.28)

project(Auxid C CXX)

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

if(EMSCRIPTEN)
    add_compile_options(
        -msimd128
        -pthread
    )

    add_link_options(
        -pthread
        -sPTHREAD_POOL_SIZE=4
        -sPROXY_TO_PTHREAD
        -sEXIT_RUNTIME=1
        -sALLOW_MEMORY_GROWTH=1
        -sINITIAL_MEMORY=64MB
    )
endif()

add_subdirectory(src)

if(Auxid_BUILD_TESTS)
  add_subdirectory(tests)
endif()
