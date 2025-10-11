cmake_minimum_required(VERSION 3.20)
project(terrain_engine)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(include)

add_executable(terrain_engine main.cpp src/chunk.cpp src/world.cpp)

target_include_directories(terrain_engine PRIVATE src)

target_link_libraries(terrain_engine include)
