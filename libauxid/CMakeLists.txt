set(SRC_FILES
  "src/cpp/auxid.cpp"

  "src/cpp/vendor/rpmalloc/rpmalloc.c"
  "src/cpp/vendor/tinycthread/tinycthread.c"
)

add_library(auxid-rt STATIC ${SRC_FILES})

target_include_directories(auxid-rt PUBLIC
  "include"
)

target_include_directories(auxid-rt PRIVATE
  "src/hpp"
)
