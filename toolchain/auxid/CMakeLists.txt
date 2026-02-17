set(SRC_FILES
  "cpp/cli.cpp"
  "cpp/main.cpp"

  "cpp/vendor/cofyc/argparse.c"
)

add_executable(auxid ${SRC_FILES})

target_link_libraries(auxid PRIVATE
  auxid-workspace
)

target_include_directories(auxid PRIVATE
  "hpp"
)
