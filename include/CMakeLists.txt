add_subdirectory(glfw-3.4)

add_library(include glad/src/glad.c)
target_include_directories(include PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/glad/include
  ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(include glfw)
