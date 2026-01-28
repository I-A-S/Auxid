cmake_minimum_required(VERSION 3.20)
project(AuxidValidator)

add_subdirectory(IACrux)

crux_setup_project()

include(CMake/FindDeps.cmake)

add_subdirectory(Src/)
