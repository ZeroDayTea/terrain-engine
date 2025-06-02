#include <iostream>
#include <random>
#include <fstream>
#include <sstream>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h"
#include "init.h"
#include "init_shaders.h"

float deltaTime = 0.0f;
float lastFrame = 0.0f;

// position and noise value at any point
struct Point {
  glm::vec3 pos;
  float value;
};
float isolevel = 0.0f;

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
}

int main() {
    GLFWwindow* window = glfw_initialization();
    if (!window) {
      return -1;
    }
    
    unsigned int shaderProgram = generate_shader_program();

    Point cube_corners[8];
    
    float s = 0.5f;
    cube_corners[0] = {glm::vec3(s, s, s), -0.5f};
    cube_corners[1] = {glm::vec3(s, s, -s), -0.5f};
    cube_corners[2] = {glm::vec3(s, -s, s), -0.5f};
    cube_corners[3] = {glm::vec3(s, -s, -s), -0.5f};
    cube_corners[4] = {glm::vec3(-s, s, s), 0.5f};
    cube_corners[5] = {glm::vec3(-s, s, -s), 0.5f};
    cube_corners[6] = {glm::vec3(-s, -s, s), 0.5f};
    cube_corners[7] = {glm::vec3(-s, -s, -s), 0.5f};

    // Setting Up VBO and VAO
    // NDC coordinates
    float vertices[] = {
      -0.5f, -0.5f, 0.0f,
      0.5f, -0.5f, 0.0f,
      0.0f, 0.5f, 0.0f
    };
    
    // world space positions
    glm::vec3 trianglePositions[] = {
      glm::vec3(0.0f, 0.0f, 0.0f)
    };

    unsigned int VBO, VAO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // position attribute 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glUseProgram(shaderProgram);

    // render loop
    while (!glfwWindowShouldClose(window)) {
      float currentFrame = static_cast<float>(glfwGetTime());
      deltaTime = currentFrame - lastFrame;
      lastFrame = currentFrame;

      processInput(window);
      
      glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glUseProgram(shaderProgram);
     
      glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
      glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);

      glm::mat4 view = camera.GetViewMatrix();
      glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &view[0][0]);

      // render triangle
      glBindVertexArray(VAO);
      for (unsigned int i = 0; i < 1; i++) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, trianglePositions[i]);
        float angle = 20.0f * i;
        model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &model[0][0]);

        glDrawArrays(GL_TRIANGLES, 0, 3);
      }
      
      glfwSwapBuffers(window);
      glfwPollEvents();
    }

    // cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}
