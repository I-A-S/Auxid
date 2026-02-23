set(SRC_FILES
    "cpp/main.cpp"

    "cpp/memory/arena.cpp"
    "cpp/memory/heap.cpp"
    "cpp/core/result.cpp"
    "cpp/thread/thread.cpp"

    "cpp/containers/vec.cpp"
    "cpp/containers/string.cpp"
    "cpp/containers/option.cpp"
    "cpp/containers/hash_map.cpp"
    "cpp/containers/hash_set.cpp"
    "cpp/containers/pair.cpp"
)

add_executable(TestSuite ${SRC_FILES})

target_link_libraries(TestSuite PRIVATE libauxid)

add_test(NAME TestSuite COMMAND TestSuite)
