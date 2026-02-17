set(SRC_FILES
  "src/cpp/platform.cpp"
)

add_library(auxid-platform STATIC ${SRC_FILES})

target_include_directories(auxid-platform PUBLIC
  include
)

target_include_directories(auxid-platform PUBLIC
  src/hpp
)

target_link_libraries(auxid-platform PUBLIC
  curl  
  libauxid
)

target_precompile_headers(auxid-platform PUBLIC 
  "include/platform/pch.hpp"
)
