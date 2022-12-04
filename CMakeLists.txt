cmake_minimum_required(VERSION 3.0)
project(threadpool)

set(CMAKE_CXX_STANDARD 20)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
include_directories(${PROJECT_SOURCE_DIR}/include)

add_subdirectory(src)