cmake_minimum_required(VERSION 3.20)
project(undaunted_terrain_game_engine)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(include)

add_executable(undaunted_terrain_game_engine main.cpp src/chunk.cpp)

target_include_directories(undaunted_terrain_game_engine PRIVATE src)

target_link_libraries(undaunted_terrain_game_engine include)

