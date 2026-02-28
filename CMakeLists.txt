cmake_minimum_required(VERSION 3.15)
project(EllipseBenchmark)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(ellipse_benchmark
    src/main.cpp
)

target_include_directories(ellipse_benchmark PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
