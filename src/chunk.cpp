#include "chunk.h"

#include "glm/geometric.hpp"
#include "marching_cubes.h"

#include <cmath>
#include <iostream>
#include <map>
#include <random>
#include <vector>

#include <glad/glad.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace {
  struct Vertex {
    glm::vec4 pos;
    glm::vec4 norm;
  };

  struct IndirectDraw {
    GLuint v_count;
    GLuint instanceCount;
    GLuint first;
    GLuint baseInstance;
  };
};

// initialize chunk data and generate chunk mesh with compute shaders
Chunk::Chunk(glm::vec3 chunkPosition, unsigned int densityShader, unsigned int mcCountShader, unsigned int mcEmitShader) : chunkPos(chunkPosition), VAO(0), vertexSSBO(0), indirectBuffer(0), offsetsSSBO(0) {
  generateMeshGPU(densityShader, mcCountShader, mcEmitShader);
}

// cleanup
Chunk::~Chunk() {
  if (VAO) {
    glDeleteVertexArrays(1, &VAO);
  }
  if (vertexSSBO) {
    glDeleteBuffers(1, &vertexSSBO);
  }
  if (indirectBuffer) {
    glDeleteBuffers(1, &indirectBuffer);
  }
  if (offsetsSSBO) {
    glDeleteBuffers(1, &offsetsSSBO);
  }
}

/*
** create gpu buffers for processing with marching cubes compute shaders
** run compute shader that generate density values for each point in the chunk
** run marching cubes compute shader over the generated density values
** read vertex count back from gpu to cpu and keep vertex buffer for rendering
*/
void Chunk::generateMeshGPU(unsigned int densityShader, unsigned int mcCountShader, unsigned int mcEmitShader) {
    // temporary OpenGL buffer objects
    unsigned int densitySSBO, triTableSSBO, edgeTableSSBO, counterSSBO;

    // density field storage
    glGenBuffers(1, &densitySSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, densitySSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_POINTS * sizeof(float), NULL, GL_STATIC_DRAW);

    // marching cubes triangle lookup table
    glGenBuffers(1, &triTableSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, triTableSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(triTable), &triTable[0], GL_STATIC_DRAW);

    // marching cubes edge lookup table
    glGenBuffers(1, &edgeTableSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, edgeTableSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(edgeTable), &edgeTable[0], GL_STATIC_DRAW);

    // per-voxel vertex offsets
    const size_t NUM_VOXELS = CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH;
    glGenBuffers(1, &offsetsSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, offsetsSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_VOXELS * sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);

    // global vertex counter
    glGenBuffers(1, &counterSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, counterSSBO);
    GLuint zero = 0;
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint), &zero, GL_DYNAMIC_READ);

    // run density compute shader
    glUseProgram(densityShader);
    glUniform3fv(glGetUniformLocation(densityShader, "chunkWorldPos"), 1, &chunkPos[0]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, densitySSBO);
    glDispatchCompute((CHUNK_WIDTH + 1 + 7) / 8, (CHUNK_HEIGHT + 1 + 7) / 8, (CHUNK_DEPTH + 1 + 7) / 8);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // marching cubes first pass
    glUseProgram(mcCountShader);
    glUniform1f(glGetUniformLocation(mcCountShader, "isolevel"), 0.0f);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, densitySSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, triTableSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, edgeTableSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, counterSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, offsetsSSBO);

    glDispatchCompute(CHUNK_WIDTH / 8, CHUNK_HEIGHT / 8, CHUNK_DEPTH / 8);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    GLuint totalVertices = 0;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, counterSSBO);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint), &totalVertices);

    if (totalVertices == 0) {
      totalVertices = 0;
    }

    glGenBuffers(1, &vertexSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Vertex) * (size_t)totalVertices, nullptr, GL_DYNAMIC_DRAW);

    // marching cubes second pass
    glUseProgram(mcEmitShader);
    glUniform1f(glGetUniformLocation(mcEmitShader, "isolevel"), 0.0f);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, densitySSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, triTableSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, edgeTableSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, vertexSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, offsetsSSBO);

    glDispatchCompute(CHUNK_WIDTH / 8, CHUNK_HEIGHT / 8, CHUNK_DEPTH / 8);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

    glGenBuffers(1, &indirectBuffer);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBuffer);
    IndirectDraw cmd = { totalVertices, 1u, 0u, 0u };
    glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(cmd), &cmd, GL_DYNAMIC_DRAW);

    setupVAO();

    glDeleteBuffers(1, &densitySSBO);
    glDeleteBuffers(1, &triTableSSBO);
    glDeleteBuffers(1, &edgeTableSSBO);
    glDeleteBuffers(1, &counterSSBO);
}

void Chunk::setupVAO() {
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Bind the generated vertex data as the source for vertex attributes
    glBindBuffer(GL_ARRAY_BUFFER, vertexSSBO);

    // Attribute 0: Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
    glEnableVertexAttribArray(0);

    // Attribute 1: Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, norm));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void Chunk::renderRaw() {
  glBindVertexArray(this->VAO);
  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBuffer);
  glDrawArraysIndirect(GL_TRIANGLES, 0);
  glBindVertexArray(0);
  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
}

void Chunk::render(unsigned int shaderProgram) {
  static GLint uModel = -1;
  if (uModel == -1) uModel = glGetUniformLocation(shaderProgram, "model");
  glm::mat4 model = glm::translate(glm::mat4(1.0f), chunkPos);
  // float angle = 20.0f * i;
  // model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f,
  // 0.5f));
  glUniformMatrix4fv(uModel, 1, GL_FALSE, &model[0][0]);

  glBindVertexArray(this->VAO);
  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBuffer);

  glDrawArraysIndirect(GL_TRIANGLES, 0);

  glBindVertexArray(0);
  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
}
