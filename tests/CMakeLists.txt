set(SRC_FILES
    "cpp/main.cpp"

    "cpp/memory/arena.cpp"
    "cpp/memory/heap.cpp"
    "cpp/memory/foreign_thread.cpp"
    "cpp/core/result.cpp"
    "cpp/thread/thread.cpp"

    "cpp/containers/vec.cpp"
    "cpp/containers/string.cpp"
    "cpp/containers/format.cpp"
    "cpp/containers/option.cpp"
    "cpp/containers/hash_base.cpp"
    "cpp/containers/hash_map.cpp"
    "cpp/containers/hash_set.cpp"
    "cpp/containers/pair.cpp"
    "cpp/containers/iterator_concepts.cpp"
    "cpp/containers/slot_map.cpp"
)

add_executable(TestSuite ${SRC_FILES})
target_precompile_headers(TestSuite PRIVATE
    <new>
    <vector>
)
target_link_libraries(TestSuite PRIVATE libauxid)
if(Auxid_BUILD_SHARED AND WIN32)
    add_custom_command(TARGET TestSuite POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:TestSuite>
            $<TARGET_FILE_DIR:TestSuite>
        COMMAND_EXPAND_LISTS
    )
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND WIN32)
    target_link_libraries(TestSuite PRIVATE stdc++exp)
endif()
set_target_properties(TestSuite PROPERTIES CXX_SCAN_FOR_MODULES ON)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_options(TestSuite PRIVATE "LINKER:--allow-multiple-definition")
endif()
add_test(NAME TestSuite COMMAND TestSuite)
