if(MSVC)
    set(AUXID_CXX_FLAGS_NO_EXCEPTIONS "/EHs-c-")
    set(AUXID_CXX_FLAGS_NO_EXCEPTIONS "${AUXID_CXX_FLAGS_NO_EXCEPTIONS};/we4530")
else()
    set(AUXID_CXX_FLAGS_NO_EXCEPTIONS "-fno-exceptions")
endif()

set(AUXID_CXX_FLAGS_INTERNAL "${AUXID_CXX_FLAGS_NO_EXCEPTIONS}" CACHE INTERNAL "Auxid compiler flags")
set(AUXID_LINK_FLAGS_INTERNAL "" CACHE INTERNAL "Auxid linker flags")

set(SRC_FILES
        "cpp/auxid.cpp"
        "cpp/logger.cpp"
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

target_compile_options(libauxid PUBLIC  ${AUXID_CXX_FLAGS_NO_EXCEPTIONS})
target_link_options(libauxid    PRIVATE ${AUXID_LINK_FLAGS_INTERNAL})

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(libauxid PRIVATE -Wno-TU-local-entity-exposure)
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
