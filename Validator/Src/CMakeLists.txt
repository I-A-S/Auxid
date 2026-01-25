set(SRC_FILES
    cpp/Main.cpp
    cpp/Validator.cpp
)

add_executable(auxid-validator ${SRC_FILES})

target_link_libraries(auxid-validator
    PRIVATE
    clang-cpp
    LLVM
)

target_include_directories(auxid-validator PRIVATE
    "hpp"
    "../../Include"
    ${LLVM_INCLUDE_DIRS}
    ${CLANG_INCLUDE_DIRS}
)

target_link_directories(auxid-validator PRIVATE
    ${LLVM_LIBRARY_DIRS}
    ${CLANG_LIBRARY_DIRS}
)

target_compile_definitions(auxid-validator PRIVATE
    ${LLVM_DEFINITIONS}
)

target_precompile_headers(auxid-validator PRIVATE hpp/PCH.hpp)
