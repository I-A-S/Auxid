set(SRC_FILES
  "src/cpp/workspace.cpp"
)

add_library(auxid-workspace STATIC ${SRC_FILES})

target_include_directories(auxid-workspace PUBLIC
  include
)

target_include_directories(auxid-workspace PUBLIC
  src/hpp
)

target_link_libraries(auxid-workspace PUBLIC
  auxid-platform
)
