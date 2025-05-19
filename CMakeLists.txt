cmake_minimum_required(VERSION 3.20)
project(undaunted_terrain_game_engine)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(include)

add_executable(undaunted_terrain_game_engine main.cpp)

target_link_libraries(undaunted_terrain_game_engine include)
