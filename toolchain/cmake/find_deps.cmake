include(FetchContent)

find_package(Clang REQUIRED CONFIG)
find_package(LLVM REQUIRED CONFIG)

if(${Clang_VERSION} VERSION_LESS 21)
    message(FATAL_ERROR "Found Clang ${Clang_VERSION}, but version 21 or higher is required.")
endif()

if(${LLVM_VERSION} VERSION_LESS 21)
    message(FATAL_ERROR "Found LLVM ${LLVM_VERSION}, but version 21 or higher is required.")
endif()

if(MSVC)
    find_path(DIA_SDK_ROOT_DIR include/dia2.h
        HINTS "$ENV{VSINSTALLDIR}/DIA SDK"
              "C:/Program Files/Microsoft Visual Studio/2022/Enterprise/DIA SDK"
              "C:/Program Files/Microsoft Visual Studio/2022/Community/DIA SDK"
              "C:/Program Files (x86)/Microsoft Visual Studio/2019/Professional/DIA SDK"
    )
    find_library(DIA_GUIDS_LIBRARY diaguids.lib HINTS "${DIA_SDK_ROOT_DIR}/lib/amd64")
    
    if(NOT DIA_GUIDS_LIBRARY)
        message(WARNING "Could not find local diaguids.lib. Linking might fail if LLVM needs it.")
    endif()

    set(LLVM_TARGETS_TO_SCRUB ${LLVM_AVAILABLE_LIBS} LLVMDebugInfoPDB LLVMDebugInfoCodeView)

    foreach(TGT ${LLVM_TARGETS_TO_SCRUB})
        if(TARGET ${TGT})
            macro(scrub_llvm_property PROP_NAME)
                get_target_property(DEPS ${TGT} ${PROP_NAME})
                if(DEPS)
                    string(REGEX MATCH ".*diaguids.*" FOUND_BAD_LIB "${DEPS}")
                    
                    if(FOUND_BAD_LIB)
                        list(FILTER DEPS EXCLUDE REGEX ".*diaguids.*")
                        
                        if(DIA_GUIDS_LIBRARY)
                            list(APPEND DEPS "${DIA_GUIDS_LIBRARY}")
                        endif()

                        set_target_properties(${TGT} PROPERTIES ${PROP_NAME} "${DEPS}")
                    endif()
                endif()
            endmacro()

            scrub_llvm_property(INTERFACE_LINK_LIBRARIES)
            scrub_llvm_property(IMPORTED_LINK_INTERFACE_LIBRARIES)
            scrub_llvm_property(IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE)
            scrub_llvm_property(IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG)
        endif()
    endforeach()
    
    message(STATUS "LLVM Targets scrubbed of absolute diaguids.lib paths.")
endif()

