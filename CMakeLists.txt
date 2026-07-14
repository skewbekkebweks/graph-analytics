cmake_minimum_required(VERSION 3.16)
project(graph_analytics CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(Threads REQUIRED)

add_executable(gra
    src/main.cpp
    src/preprocess.cpp
    src/csv_reader.cpp
    src/graph_store.cpp
    src/leaderrank.cpp)

target_include_directories(gra PRIVATE src)
target_link_libraries(gra PRIVATE Threads::Threads)
target_compile_options(gra PRIVATE -O3 -Wall -Wextra)
