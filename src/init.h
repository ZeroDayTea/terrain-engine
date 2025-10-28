#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Camera;

extern const unsigned int SCREEN_WDITH;
extern const unsigned int SCREEN_HEIGHT;

// callbacks
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// initialization
GLFWwindow* glfw_initialization();

// camera accessor
Camera& get_camera();
