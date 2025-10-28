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
  struct IndirectDraw {
    GLuint v_count;
    GLuint instanceCount;
    GLuint first;
    GLuint baseInstance;
  };
};

// initialize chunk data and generate chunk mesh with compute shaders
Chunk::Chunk(glm::vec3 chunkPosition) : chunkPos(chunkPosition), VAO(0), vertexSSBO(0), indirectBuffer(0) { }

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
}

void Chunk::adoptPrebuilt(GLuint vao_, GLuint vbo_, GLuint indirect_) {
  if (VAO) {
    glDeleteVertexArrays(1, &VAO);
  }
  if (vertexSSBO) {
    glDeleteBuffers(1, &vertexSSBO);
  }
  if (indirectBuffer) {
    glDeleteBuffers(1, &indirectBuffer);
  }

  VAO = vao_;
  vertexSSBO = vbo_;
  indirectBuffer = indirect_;
}

void Chunk::renderRaw() {
  glBindVertexArray(this->VAO);
  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBuffer);
  glDrawArraysIndirect(GL_TRIANGLES, 0);
  glBindVertexArray(0);
  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
}
