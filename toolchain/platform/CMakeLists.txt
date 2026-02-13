set(SRC_FILES
  "cpp/platform.cpp"
)

add_library(auxid-platform STATIC ${SRC_FILES})

target_include_directories(auxid-platform PUBLIC
  hpp/
)

target_link_libraries(auxid-platform PUBLIC
  auxid-rt
)

target_precompile_headers(auxid-platform PUBLIC 
  "hpp/pch.hpp"
)
