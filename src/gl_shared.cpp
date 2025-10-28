#include "gl_shared.h"
#include <GLFW/glfw3.h>

GLFWwindow* create_shared_context(GLFWwindow* main_window) {
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if __APPLE__
    lfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLFWwindow* worker = glfwCreateWindow(1, 1, "worker", nullptr, main_window);
    glfwDefaultWindowHints();
    return worker;
}

