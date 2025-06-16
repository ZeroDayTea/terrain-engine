#include <iostream>
#include <random>
#include <fstream>
#include <sstream>
#include <string>
#include <random>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h"
#include "init.h"
#include "init_shaders.h"
#include "marching_cubes.h"

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
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
}

glm::vec3 interpolate_vertices(const Point& p1, const Point& p2) {
    if (std::abs(p1.value - p2.value) < 0.00001f) {
      return p1.pos;
    }
    float t = (isolevel - p1.value) / (p2.value - p1.value);
    return p1.pos + t * (p2.pos - p1.pos);
}

int main() {
    GLFWwindow* window = glfw_initialization();
    if (!window) {
      return -1;
    }
    
    unsigned int shaderProgram = generate_shader_program();

    // temp randomness
    std::mt19937 rng(std::random_device{}());
    float min_val = -0.5f;
    float max_val = 0.5f;
    std::uniform_real_distribution<float> dist(min_val, max_val);
   
    // 0: bottom-left-front
    // 1: bottom-right-front
    // 2: bottom-right-back
    // 3: bottom-left-back
    // 4: top-left-front
    // 5: top-right-front
    // 6: top-right-back
    // 7: top-left-back
    float cubeSize = 1.0f;
    glm::vec3 cornerOffsets[8] = {
      glm::vec3(0.0f, 0.0f, 0.0f),
      glm::vec3(1.0f, 0.0f, 0.0f),
      glm::vec3(1.0f, 0.0f, 1.0f),
      glm::vec3(0.0f, 0.0f, 1.0f),
      glm::vec3(0.0f, 1.0f, 0.0f),
      glm::vec3(1.0f, 1.0f, 0.0f),
      glm::vec3(1.0f, 1.0f, 1.0f),
      glm::vec3(0.0f, 1.0f, 1.0f)
    };
    std::vector<glm::vec3> triangles;
    for(size_t x = 0; x < 8; x++) {
      for(size_t y = 0; y < 8; y++) {
        for(size_t z = 0; z < 8; z++) {
          Point cubeCorners[8];
          // NDC coordinates of cube (generate locally)
          glm::vec3 cubePos = glm::vec3(x, y, z) * cubeSize;
          for(size_t i = 0; i < 8; i++) {
            cubeCorners[i].pos = cubePos + (cornerOffsets[i] * cubeSize);
            cubeCorners[i].value = dist(rng);
          }

          unsigned int cubePattern = 0;
          for(int i = 0; i < 8; i++) {
            if (cubeCorners[i].value < isolevel) cubePattern |= 1 << i;
          }

          // given an edge, which two points it connects
          int cornersFromEdge[12][2] = {
            // edge 0
            {0, 1},
            // edge 1
            {1, 2},
            // edge 2
            {2, 3},
            // edge 3
            {3, 0},
            // edge 4
            {4, 5},
            // edge 5
            {5, 6},
            // edge 6
            {6, 7},
            // edge 7
            {7, 4},
            // edge 8
            {0, 4},
            // edge 9
            {1, 5},
            // edge 10
            {2, 6},
            // edge 11
            {3, 7}
          };
          
          int cubeMask = edgeTable[cubePattern];
          if (cubeMask != 0) {
            glm::vec3 interpolatedEdges[12];
            for(int i = 0; i < 12; i++) {
              Point p1 = cubeCorners[cornersFromEdge[i][0]];
              Point p2 = cubeCorners[cornersFromEdge[i][1]];
              if (cubeMask & (1 << i)) interpolatedEdges[i] = interpolate_vertices(p1, p2);
            }
            
            int* edges = triTable[cubePattern];
            int i = 0;
            while(edges[i] != -1) {
              triangles.push_back(interpolatedEdges[edges[i]]);
              triangles.push_back(interpolatedEdges[edges[i+1]]);
              triangles.push_back(interpolatedEdges[edges[i+2]]);
              i += 3;
            }
          }
      }
    }
  }

    // Setting Up VBO and VAO
    std::vector<float> vertices;
    for (const glm::vec3 vert : triangles) {
      vertices.push_back(vert.x);
      vertices.push_back(vert.y);
      vertices.push_back(vert.z);
    }

    // world space positions
    glm::vec3 trianglePositions[] = {
      glm::vec3(0.0f, 0.0f, 0.0f)
    };

    unsigned int VBO, VAO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
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

        glDrawArrays(GL_TRIANGLES, 0, triangles.size());
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
