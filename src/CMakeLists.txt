if(MSVC)
    set(AUXID_CXX_FLAGS_NO_EXCEPTIONS "/EHs-c-")
    set(AUXID_CXX_FLAGS_NO_EXCEPTIONS "${AUXID_CXX_FLAGS_NO_EXCEPTIONS};/wd4530")
    set(AUXID_CXX_FLAGS_NO_RTTI "/GR-")
else()
    set(AUXID_CXX_FLAGS_NO_EXCEPTIONS "-fno-exceptions")
    set(AUXID_CXX_FLAGS_NO_RTTI "-fno-rtti")
endif()

set(AUXID_CXX_FLAGS_INTERNAL "${AUXID_CXX_FLAGS_NO_EXCEPTIONS};${AUXID_CXX_FLAGS_NO_RTTI}" CACHE INTERNAL "Auxid compiler flags")
set(AUXID_LINK_FLAGS_INTERNAL "" CACHE INTERNAL "Auxid linker flags")

set(SRC_FILES)

if(NOT AUXID_USE_SYSTEM_MALLOC)
    list(APPEND SRC_FILES "c/vendor/rpmalloc/rpmalloc.c")
endif()

set(AUXID_IMPL_MODULES
    cpp/auxid-core.cpp
    cpp/auxid-containers.cpp
    cpp/auxid-thread.cpp
    cpp/auxid-time.cpp
    cpp/auxid-env.cpp
    cpp/auxid-ipc.cpp
    cpp/auxid-test.cpp
)

if(Auxid_BUILD_SHARED)
    set(_AUXID_LIB_TYPE SHARED)
else()
    set(_AUXID_LIB_TYPE STATIC)
endif()

add_library(libauxid ${_AUXID_LIB_TYPE} ${SRC_FILES} ${AUXID_IMPL_MODULES})
message(STATUS "Auxid: libauxid library type = ${_AUXID_LIB_TYPE}")

if(NOT WIN32)
    set_target_properties(libauxid PROPERTIES OUTPUT_NAME auxid)
endif()

if(Auxid_BUILD_SHARED)
    target_compile_definitions(libauxid
        PUBLIC  AUXID_SHARED_BUILD=1
        PRIVATE AUXID_BUILDING_LIBRARY=1
    )
    if(WIN32)
        set_target_properties(libauxid PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
    else()
        set_target_properties(libauxid PROPERTIES
            CXX_VISIBILITY_PRESET hidden
            C_VISIBILITY_PRESET hidden
            VISIBILITY_INLINES_HIDDEN ON
        )
    endif()
endif()

if(AUXID_USE_SYSTEM_MALLOC)
    target_compile_definitions(libauxid PUBLIC AUXID_USE_SYSTEM_MALLOC=1)
endif()

target_sources(libauxid
    PUBLIC FILE_SET auxid_interfaces TYPE CXX_MODULES
        BASE_DIRS "${AUXID_ROOT}/src/ixx"
        FILES
            "${AUXID_ROOT}/src/ixx/auxid.ixx"
            "${AUXID_ROOT}/src/ixx/auxid-core.ixx"
            "${AUXID_ROOT}/src/ixx/auxid-memory.ixx"
            "${AUXID_ROOT}/src/ixx/auxid-containers.ixx"
            "${AUXID_ROOT}/src/ixx/auxid-thread.ixx"
            "${AUXID_ROOT}/src/ixx/auxid-fs.ixx"
            "${AUXID_ROOT}/src/ixx/auxid-time.ixx"
            "${AUXID_ROOT}/src/ixx/auxid-env.ixx"
            "${AUXID_ROOT}/src/ixx/auxid-ipc.ixx"
            "${AUXID_ROOT}/src/ixx/auxid-test.ixx"
)
set_target_properties(libauxid PROPERTIES CXX_SCAN_FOR_MODULES ON)

if(MSVC)
    foreach(_cfg CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_MINSIZEREL)
        if(DEFINED ${_cfg})
            string(REPLACE "/EHsc" "" ${_cfg} "${${_cfg}}")
            string(REGEX REPLACE "/GR( |$)" " " ${_cfg} "${${_cfg}}")
        endif()
    endforeach()
endif()

include(GNUInstallDirs)

target_include_directories(libauxid PUBLIC
    "$<BUILD_INTERFACE:${AUXID_ROOT}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_include_directories(libauxid PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/h/vendor"
        "hpp"
)

target_compile_options(libauxid PUBLIC  ${AUXID_CXX_FLAGS_NO_EXCEPTIONS} ${AUXID_CXX_FLAGS_NO_RTTI})
target_link_options(libauxid    PRIVATE ${AUXID_LINK_FLAGS_INTERNAL})

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(libauxid PRIVATE -Wno-TU-local-entity-exposure)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(libauxid PRIVATE
        -Wno-subobject-linkage
    )
    set_source_files_properties(
        cpp/auxid-test.cpp
        
        TARGET libauxid
        PROPERTIES
            COMPILE_OPTIONS "-O2"
    )
endif()

if(MSVC AND NOT CMAKE_C_COMPILER_ID MATCHES "Clang")
    set_source_files_properties(c/vendor/rpmalloc/rpmalloc.c PROPERTIES
            COMPILE_OPTIONS "/experimental:c11atomics"
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set_source_files_properties(c/vendor/rpmalloc/rpmalloc.c PROPERTIES
            COMPILE_DEFINITIONS "_DEFAULT_SOURCE;_GNU_SOURCE"
    )
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    find_package(Threads REQUIRED)
    target_link_libraries(libauxid PUBLIC Threads::Threads)
endif()
