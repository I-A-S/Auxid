if(MSVC)
    set(AUXID_CXX_FLAGS_STANDARD "/EHs-c-")

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(AUXID_CXX_FLAGS_METAL
                "/clang:-nostdlib++"
                "/clang:-ffreestanding"
        )
        set(AUXID_LINK_FLAGS_METAL
                "/NODEFAULTLIB:libcpmt.lib"
                "/NODEFAULTLIB:msvcprt.lib"
        )
    else()
        set(AUXID_CXX_FLAGS_METAL "")
        set(AUXID_LINK_FLAGS_METAL "")
        message(WARNING "auxid_platform_close_to_metal is limited under native MSVC; skipping Clang-only freestanding flags.")
    endif()
else()
    set(AUXID_CXX_FLAGS_STANDARD "-fno-exceptions")

    set(AUXID_CXX_FLAGS_METAL
            "-nostdlib++"
            "-ffreestanding"
    )
    set(AUXID_LINK_FLAGS_METAL
            "-nostdlib++"
    )
endif()

set(AUXID_CXX_FLAGS_INTERNAL "${AUXID_CXX_FLAGS_STANDARD};${AUXID_CXX_FLAGS_METAL}" CACHE INTERNAL "Auxid compiler flags")
set(AUXID_LINK_FLAGS_INTERNAL "${AUXID_LINK_FLAGS_METAL}" CACHE INTERNAL "Auxid linker flags")

add_library(auxid_platform_standard INTERFACE)
target_compile_options(auxid_platform_standard INTERFACE ${AUXID_CXX_FLAGS_STANDARD})

add_library(auxid_platform_close_to_metal INTERFACE)
target_link_libraries(auxid_platform_close_to_metal INTERFACE auxid_platform_standard)
target_compile_options(auxid_platform_close_to_metal INTERFACE ${AUXID_CXX_FLAGS_METAL})
target_link_options(auxid_platform_close_to_metal INTERFACE ${AUXID_LINK_FLAGS_METAL})

set(SRC_FILES
        "cpp/auxid.cpp"
        "cpp/logger.cpp"
        "cpp/vendor/rpmalloc/rpmalloc.c"
        "cpp/vendor/tinycthread/tinycthread.c"
)

add_library(libauxid STATIC ${SRC_FILES})

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
        "hpp"
)

target_compile_options(libauxid PRIVATE ${AUXID_CXX_FLAGS_INTERNAL})
target_link_options(libauxid PRIVATE ${AUXID_LINK_FLAGS_INTERNAL})

if(MSVC AND NOT CMAKE_C_COMPILER_ID MATCHES "Clang")
    set_source_files_properties(cpp/vendor/rpmalloc/rpmalloc.c PROPERTIES
            COMPILE_OPTIONS "/experimental:c11atomics"
    )
elseif(UNIX)
    set_source_files_properties(cpp/vendor/rpmalloc/rpmalloc.c PROPERTIES
            COMPILE_DEFINITIONS "_DEFAULT_SOURCE;_GNU_SOURCE"
    )
endif()
