set(CMAKE_SYSTEM_NAME Emscripten)
set(CMAKE_SYSTEM_PROCESSOR wasm32)

include("$ENV{EMSDK}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake")

string(APPEND CMAKE_C_FLAGS " -msimd128 -pthread")
string(APPEND CMAKE_CXX_FLAGS " -msimd128 -pthread")
