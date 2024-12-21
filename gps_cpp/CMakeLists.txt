cmake_minimum_required(VERSION 3.30.5)

project(GPS_Reproduction)

set(CMAKE_CXX_STANDARD 17)
find_package(TBB REQUIRED)
include_directories(include)

add_subdirectory(lib)
add_subdirectory(src)