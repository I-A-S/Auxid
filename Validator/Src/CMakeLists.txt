set(SRC_FILES
    cpp/Main.cpp
    cpp/Validator.cpp
)

add_executable(oxide-validator ${SRC_FILES})

target_link_libraries(oxide-validator
    PRIVATE
    clang-cpp
    LLVM
)

target_include_directories(oxide-validator PRIVATE
    "hpp"
    "../../Include"
    ${LLVM_INCLUDE_DIRS}
    ${CLANG_INCLUDE_DIRS}
)

target_link_directories(oxide-validator PRIVATE
    ${LLVM_LIBRARY_DIRS}
    ${CLANG_LIBRARY_DIRS}
)

target_compile_definitions(oxide-validator PRIVATE
    ${LLVM_DEFINITIONS}
)
