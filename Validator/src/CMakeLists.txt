set(SRC_FILES
    cpp/main.cpp

    cpp/raw_type_police.cpp
    cpp/use_after_move.cpp
    cpp/use_after_free.cpp
    
    cpp/violation_reporter.cpp
)

add_executable(auxid-validator ${SRC_FILES})

target_link_libraries(auxid-validator PRIVATE
    IACrux
    Fixpoint
)

target_include_directories(auxid-validator PRIVATE
    hpp
)
