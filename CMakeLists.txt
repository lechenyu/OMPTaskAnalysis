project(OMPTaskAnalysis)
cmake_minimum_required(VERSION 3.12)
enable_language(CXX)
set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_CXX_STANDARD 14)
find_package(OpenMP REQUIRED)

add_subdirectory(src)
add_subdirectory(test)