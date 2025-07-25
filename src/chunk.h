#pragma once

#include <glm/glm.hpp>
#include <vector>

class Chunk {
public:
  // chunk size constants
  static constexpr int CHUNK_WIDTH = 32;
  static constexpr int CHUNK_HEIGHT = 32;
  static constexpr int CHUNK_DEPTH = 32;

  static constexpr int NUM_POINTS = (CHUNK_WIDTH + 1) * (CHUNK_HEIGHT + 1) * (CHUNK_DEPTH + 1);
  static constexpr int MAX_VERTICES = CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH * 5 * 3; // max 5 triangles per chunk with 3 vertices each

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
  unsigned int indirectBuffer;

  void generateMeshGPU(unsigned int densityShader, unsigned int marchingShader);
  void checkVertexCount();

  void setupVAO();
};
