cmake_minimum_required(VERSION 3.20)
project(terrain_engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(include)

set(SOURCES
    main.cpp
    src/chunk.cpp
    src/world.cpp
    src/init.cpp
    src/gl_shared.cpp
    src/frustum.cpp
    src/load_shaders.cpp
    src/chunk_worker.cpp)

add_executable(terrain_engine ${SOURCES})
target_include_directories(terrain_engine PRIVATE src)
target_link_libraries(terrain_engine include)
