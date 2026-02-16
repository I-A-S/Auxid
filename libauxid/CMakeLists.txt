set(SRC_FILES
  "src/cpp/auxid.cpp"

  "src/cpp/vendor/rpmalloc/rpmalloc.c"
  "src/cpp/vendor/tinycthread/tinycthread.c"
)

add_library(libauxid STATIC ${SRC_FILES})

target_include_directories(libauxid PUBLIC
  "include"
)

target_include_directories(libauxid PRIVATE
  "src/hpp"
)
