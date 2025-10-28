#pragma once

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <vector>
#include "config.h"

class Chunk {
public:
  // chunk size constants
  static constexpr int CHUNK_WIDTH = Config::CHUNK_WIDTH;
  static constexpr int CHUNK_HEIGHT = Config::CHUNK_HEIGHT;
  static constexpr int CHUNK_DEPTH = Config::CHUNK_DEPTH;

  Chunk(glm::vec3 chunkPosition);

  ~Chunk();

  // prevent accidental copy/move
  Chunk(const Chunk&) = delete;
  Chunk& operator=(const Chunk&) = delete;
  Chunk(Chunk&&) = delete;
  Chunk& operator=(Chunk&&) = delete;

  void renderRaw();

  void adoptPrebuilt(GLuint vao, GLuint vertexSSBO, GLuint indirectBuffer);

  glm::vec3 chunkPos;

private:
  unsigned int VAO; // vertex array object
  unsigned int vertexSSBO; // vertex output buffer
  unsigned int indirectBuffer;
};
