#pragma once

#include <glm/glm.hpp>
#include <vector>

class Chunk {
public:
  // chunk size constants
  static const int CHUNK_WIDTH = 16;
  static const int CHUNK_HEIGHT = 16;
  static const int CHUNK_DEPTH = 16;

  Chunk(glm::vec3 chunkPosition, unsigned int densityShader, unsigned int marchingShader);

  ~Chunk();

  // prevent accidental copy/move
  Chunk(const Chunk&) = delete;
  Chunk& operator=(const Chunk&) = delete;
  Chunk(Chunk&&) = delete;
  Chunk& operator=(Chunk&&) = delete;

  void render(unsigned int shaderProgram);

  glm::vec3 chunkPos;

private:
  unsigned int VAO; // vertex array object
  unsigned int vertexSSBO; // vertex output buffer
  unsigned int vertexCount;

  void generateMeshGPU(unsigned int densityShader, unsigned int marchingShader);

  void setupVAO();
};
