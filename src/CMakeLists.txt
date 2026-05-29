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

set(SRC_FILES
        "cpp/auxid.cpp"
        "cpp/containers.cpp"
        "cpp/logger.cpp"
        "cpp/panic.cpp"
        "cpp/test.cpp"
)

if(NOT AUXID_USE_SYSTEM_MALLOC)
    list(APPEND SRC_FILES "c/vendor/rpmalloc/rpmalloc.c")
endif()

add_library(libauxid STATIC ${SRC_FILES})

if(AUXID_USE_SYSTEM_MALLOC)
    target_compile_definitions(libauxid PUBLIC AUXID_USE_SYSTEM_MALLOC=1)
endif()

target_sources(libauxid
    PUBLIC FILE_SET auxid_modules TYPE CXX_MODULES
        BASE_DIRS "${CMAKE_CURRENT_LIST_DIR}/modules"
        FILES
            "${CMAKE_CURRENT_LIST_DIR}/modules/auxid.cppm"
            "${CMAKE_CURRENT_LIST_DIR}/modules/auxid-core.cppm"
            "${CMAKE_CURRENT_LIST_DIR}/modules/auxid-memory.cppm"
            "${CMAKE_CURRENT_LIST_DIR}/modules/auxid-containers.cppm"
            "${CMAKE_CURRENT_LIST_DIR}/modules/auxid-thread.cppm"
            "${CMAKE_CURRENT_LIST_DIR}/modules/auxid-fs.cppm"
            "${CMAKE_CURRENT_LIST_DIR}/modules/auxid-test.cppm"
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

target_include_directories(libauxid PUBLIC
        "${AUXID_ROOT}/include"
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
    # GCC 15: -O3 causes an internal compiler error with GCC 15.2 on auxid-test.cppm (alias/nonnull on memcpy).
    # Override -O2 with -O3 on this TU only.
    set_source_files_properties(
        FILE_SET auxid_modules
        FILES "${CMAKE_CURRENT_LIST_DIR}/modules/auxid-test.cppm"
        TARGET libauxid
        PROPERTIES
            COMPILE_OPTIONS "-O2"
    )
endif()

if(MSVC AND NOT CMAKE_C_COMPILER_ID MATCHES "Clang")
    set_source_files_properties(c/vendor/rpmalloc/rpmalloc.c PROPERTIES
            COMPILE_OPTIONS "/experimental:c11atomics"
    )
elseif(UNIX)
    set_source_files_properties(c/vendor/rpmalloc/rpmalloc.c PROPERTIES
            COMPILE_DEFINITIONS "_DEFAULT_SOURCE;_GNU_SOURCE"
    )
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(Threads REQUIRED)
    target_link_libraries(libauxid PUBLIC Threads::Threads)
endif()
