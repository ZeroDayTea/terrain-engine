const unsigned int SCREEN_HEIGHT = 1080;
const unsigned int SCREEN_WIDTH = 1920;
float lastX = SCREEN_WIDTH / 2.0f;
float lastY = SCREEN_HEIGHT / 2.0f;

bool firstMouse = true;
bool cursorLocked = true;
const float MOUSE_SENSITIVITY = 0.05f;

Camera camera(glm::vec3(16.0f, 20.0f, 24.0f));

void mouse_callback(GLFWwindow *window, double xposIn, double yposIn) {
  float xpos = static_cast<float>(xposIn);
  float ypos = static_cast<float>(yposIn);

  if (firstMouse) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos; // y-coords go bottom to top

  lastX = xpos;
  lastY = ypos;

  xoffset *= MOUSE_SENSITIVITY;
  yoffset *= MOUSE_SENSITIVITY;

  camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

/* void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) { */
/*     if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) { */
/*         cursorLocked = !cursorLocked; */

/*         if (cursorLocked) { */
/*             glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); */
/*             firstMouse = true; // reset when re-locking */
/*         } else { */
/*             glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); */
/*         } */
/*     } */
/* } */

GLFWwindow *glfw_initialization() {
  // glfw initialization
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  // glfw window
  GLFWwindow *window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "engineee",
                                        nullptr, nullptr);
  if (!window) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return nullptr;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);
  // glfwSetKeyCallback(window, key_callback);
  // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // loadl opengl function pointers
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    glfwTerminate();
    std::cerr << "Failed to initialize GLAD" << std::endl;
    return nullptr;
  }

  glEnable(GL_DEPTH_TEST);

  // face culling for better performance
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

  return window;
}
