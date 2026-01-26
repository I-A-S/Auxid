cmake_minimum_required(VERSION 3.20)
project(AuxidValidator)

add_subdirectory(IACore)

iacore_setup_project()

include(CMake/FindDeps.cmake)

add_subdirectory(Src/)
