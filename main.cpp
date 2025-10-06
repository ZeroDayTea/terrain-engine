#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h"
#include "init.h"
#include "init_shaders.h"
#include "src/init_shaders.h"
#include "world.h"
#include "frustum.h"

float deltaTime = 0.0f;
float lastFrame = 0.0f;

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) 
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
}

int main() {
    GLFWwindow* window = glfw_initialization();
    if (!window) {
      return -1;
    }

    unsigned int shaderProgram = generate_shader_program();
    unsigned int densityComputeProgram = generate_compute_program("../shaders/snoise.comp", "../shaders/density.comp");
    unsigned int mcCountComputeProgram = generate_compute_program("../shaders/mc_count.comp");
    unsigned int mcEmitComputeProgram = generate_compute_program("../shaders/mc_emit.comp");
    auto U = get_locations(shaderProgram);

    // starting a new scope so chunk destructors get called
    {
      World world(densityComputeProgram, mcCountComputeProgram, mcEmitComputeProgram);

      // sun-like lighting
      glm::vec3 lightColor(1.0f, 0.95f, 0.0f);
      glm::vec3 lightPos(24.0f, 50.0f, 24.0f);
      float renderDistance = 500.0f;
      
      // render loop
      while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        world.update(camera.Position);

        // sky blue color
        glClearColor(0.5f, 0.7f, 0.9f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glm::mat4 view = camera.GetViewMatrix();
        glUniformMatrix4fv(U.uView, 1, GL_FALSE, &view[0][0]);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, renderDistance); // setting near-plane and far-plane
        glUniformMatrix4fv(U.uProj, 1, GL_FALSE, &projection[0][0]);

        glUniform3fv(U.uLightColor, 1, &lightColor[0]);
        glUniform3fv(U.uLightPos, 1, &lightPos[0]);
        glUniform3fv(U.uViewPos, 1, &camera.Position[0]);

        glm::mat4 VP = projection * view;
        Frustum fr = make_frustum(VP);

        world.render(shaderProgram, fr, U.uModel);

        glfwSwapBuffers(window);
        glfwPollEvents();
      }
    }

    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}
