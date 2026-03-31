if(MSVC)
    set(AUXID_CXX_FLAGS_STANDARD "/EHs-c-")

    set(AUXID_CXX_FLAGS_METAL
            "/clang:-nostdlib++"
            "/clang:-ffreestanding"
    )
    set(AUXID_LINK_FLAGS_METAL
            "/NODEFAULTLIB:libcpmt.lib"
            "/NODEFAULTLIB:msvcprt.lib"
    )
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

target_include_directories(libauxid PUBLIC
        "${AUXID_ROOT}/include"
)

target_include_directories(libauxid PRIVATE
        "hpp"
)

target_compile_options(libauxid PRIVATE ${AUXID_CXX_FLAGS_INTERNAL})
target_link_options(libauxid PRIVATE ${AUXID_LINK_FLAGS_INTERNAL})
