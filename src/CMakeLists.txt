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
  "cpp/auxid.cpp"

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
