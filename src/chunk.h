#pragma once

#include <glm/glm.hpp>
#include <vector>

class Chunk {
public:
  Chunk(glm::vec3 chunkPosition);

  ~Chunk();

  void render(unsigned int shaderProgram);

  glm::vec3 chunkPos;

private:
  // chunk size constants
  static const int CHUNK_WIDTH = 16;
  static const int CHUNK_HEIGHT = 16;
  static const int CHUNK_DEPTH = 16;

  // given an edge, which two points it connects
  static const int cornersFromEdge[12][2];

  // 0: bottom-left-front
  // 1: bottom-right-front
  // 2: bottom-right-back
  // 3: bottom-left-back
  // 4: top-left-front
  // 5: top-right-front
  // 6: top-right-back
  // 7: top-left-back
  static const glm::vec3 cornerOffsets[8];

  static const float cubeSize;

  // level at which vertices are rendered or not
  static const double isolevel;

  unsigned int VAO; // vertex array object
  unsigned int VBO; // vertex buffer object

  double points[CHUNK_DEPTH][CHUNK_WIDTH][CHUNK_HEIGHT];
  std::vector<glm::vec3> triangles;
  std::vector<float> vertices;

  void generateMesh();
};
