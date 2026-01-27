include(FetchContent)

if(MSVC)
    set(Clang_DIR "$ENV{WIN_LLVM_DIR}/lib/cmake/clang")
    set(LLVM_DIR "$ENV{WIN_LLVM_DIR}/lib/cmake/llvm")
endif()

find_package(Clang REQUIRED CONFIG)
find_package(LLVM REQUIRED CONFIG)

if(MSVC)
    set(LLVM_TARGETS_TO_SCRUB ${LLVM_AVAILABLE_LIBS} LLVMDebugInfoCodeView LLVMDebugInfoPDB LLVMDebugInfoMSF)

    foreach(TGT ${LLVM_TARGETS_TO_SCRUB})
        if(TARGET ${TGT})
            get_target_property(DEPS ${TGT} INTERFACE_LINK_LIBRARIES)
            if(DEPS)
                list(FILTER DEPS EXCLUDE REGEX ".*diaguids.*")
                set_target_properties(${TGT} PROPERTIES INTERFACE_LINK_LIBRARIES "${DEPS}")
            endif()
    
            get_target_property(DEPS_DEBUG ${TGT} IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG)
            if(DEPS_DEBUG)
                list(FILTER DEPS_DEBUG EXCLUDE REGEX ".*diaguids.*")
                set_target_properties(${TGT} PROPERTIES IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG "${DEPS_DEBUG}")
            endif()
        endif()
    endforeach()
    
    message(STATUS "LLVM Targets scrubbed of absolute diaguids.lib paths.")
endif()