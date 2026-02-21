macro(auxid_setup_project)

    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "^(Clang|AppleClang)$")
        message(FATAL_ERROR "Auxid requires Clang or Clang-CL. Current compiler detected: ${CMAKE_CXX_COMPILER_ID}")
    endif()

    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

    if(CMAKE_SYSTEM_PROCESSOR MATCHES "amd64|x86_64|AMD64")
        set(AUXID_ARCH_X64 TRUE CACHE INTERNAL "")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64|ARM64")
        set(AUXID_ARCH_ARM64 TRUE CACHE INTERNAL "")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "wasm32|emscripten")
        set(AUXID_ARCH_WASM TRUE CACHE INTERNAL "")
    else()
        message(WARNING "Auxid: Unrecognized architecture '${CMAKE_SYSTEM_PROCESSOR}'. Proceed with caution.")
    endif()

endmacro()