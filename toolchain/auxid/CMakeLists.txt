set(SRC_FILES
  "cpp/main.cpp"
)

add_executable(auxid ${SRC_FILES})

target_link_libraries(auxid PRIVATE
  auxid-platform
)

target_include_directories(auxid PRIVATE
  "hpp"
)
