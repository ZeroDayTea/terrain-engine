#include "chunk.h"

#include "glm/geometric.hpp"
#include "marching_cubes.h"

// #define DB_PERLIN_IMPL
// #include "db_perlin.hpp"
#include "snoise.h"

#include <cmath>
#include <iostream>
#include <map>
#include <random>
#include <vector>

#include <glad/glad.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace {
// position and noise value at any point
struct Point {
  glm::vec3 pos;
  float value;
};

glm::vec3 interpolate_vertices(const Point &p1, const Point &p2,
                               float isolevel, bool interpolation) {
  if (interpolation) {
    if (std::abs(p1.value - p2.value) < 0.00001f) {
      return p1.pos;
    }
    float t = (isolevel - p1.value) / (p2.value - p1.value);
    return p1.pos + t * (p2.pos - p1.pos);
  }
  return (1.0f / 2.0f) * (p1.pos + p2.pos);
}

std::vector<glm::vec3> calcNormals(const std::vector<glm::vec3> &vertices) {
  std::vector<glm::vec3> normals(vertices.size(), glm::vec3(0.0f));

  // over triangles
  for (size_t i = 0; i < vertices.size(); i += 3) {
    const glm::vec3 &p1 = vertices[i];
    const glm::vec3 &p2 = vertices[i + 1];
    const glm::vec3 &p3 = vertices[i + 2];

    glm::vec3 e1 = p2 - p1;
    glm::vec3 e2 = p3 - p1;
    glm::vec3 faceNormal = glm::normalize(glm::cross(e1, e2));

    normals[i] += faceNormal;
    normals[i + 1] += faceNormal;
    normals[i + 2] += faceNormal;
  }

  for (size_t i = 0; i < normals.size(); ++i) {
    normals[i] = glm::normalize(normals[i]);
  }

  return normals;
}
}; // namespace

const int Chunk::cornersFromEdge[12][2] = {
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
    {3, 7}};

const glm::vec3 Chunk::cornerOffsets[8] = {
    glm::vec3(0, 0, 0), glm::vec3(1, 0, 0),
    glm::vec3(1, 0, 1), glm::vec3(0, 0, 1),
    glm::vec3(0, 1, 0), glm::vec3(1, 1, 0),
    glm::vec3(1, 1, 1), glm::vec3(0, 1, 1)};

const float Chunk::cubeSize = 1.0f;

const float Chunk::isolevel = 0.0f;

Chunk::Chunk(glm::vec3 chunkPosition) : chunkPos(chunkPosition) {
  generateMesh();
}

Chunk::~Chunk() {
  // cleanup
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
}

void Chunk::generateMesh() {
  for (int x = 0; x <= CHUNK_DEPTH; ++x) {
    for (int y = 0; y <= CHUNK_WIDTH; ++y) {
      for (int z = 0; z <= CHUNK_HEIGHT; ++z) {
        glm::vec3 worldPos = this->chunkPos + glm::vec3(x, y, z);
        glm::vec3 samplePos = glm::vec3(worldPos.x * 0.5f, worldPos.y * 0.25f, worldPos.z * 0.5f);

        // noise layer properties
        int octaves = 4;
        float noise = 0.0f;
        float frequency = 0.0035f;
        float amplitude = 1.0f;
        float lacunarity = 2.0f;
        float persistence = 0.5f;

        // octaves
        for (int j = 0; j < octaves; j++) {
          noise += snoise(samplePos.x * frequency, samplePos.y * frequency,
                             samplePos.z * frequency) *
                  amplitude;
          amplitude *= persistence;
          frequency *= lacunarity;
        }

        float floorOffset = 1.0f;
        float noiseWeight = 5.0f;
        float density = -(samplePos.y + floorOffset) + noise * noiseWeight;
        if (density > 0.0f) {
          density = std::pow(std::abs(density), 0.7f);
        }

        float hardFloor = 0.0f;
        float hardFloorWeight = 0.0f;
        if (worldPos.y < hardFloor) {
          density += hardFloorWeight;
        }
        this->points[x][y][z] = density;
      }
    }
  }

  for (int x = 0; x < CHUNK_DEPTH; ++x) {
    for (int y = 0; y < CHUNK_WIDTH; ++y) {
      for (int z = 0; z < CHUNK_HEIGHT; ++z) {

        Point cubeCorners[8];
        glm::vec3 cubePos = glm::vec3(x, y, z);

        for (int i = 0; i < 8; i++) {
          glm::vec3 offset = cornerOffsets[i];
          cubeCorners[i].pos = (cubePos + offset);
          // std::cout << this->points[x][y][z] << std::endl;
          cubeCorners[i].value = this->points
            [static_cast<int>(x + offset.x)]
            [static_cast<int>(y + offset.y)]
            [static_cast<int>(z + offset.z)];
        }

        unsigned int cubePattern = 0;
        for (int i = 0; i < 8; i++) {
          if (cubeCorners[i].value > isolevel)
            cubePattern |= 1 << i;
        }

        int cubeMask = edgeTable[cubePattern];
        if (cubeMask != 0) {
          glm::vec3 interpolatedEdges[12];
          for (int i = 0; i < 12; i++) {
            Point p1 = cubeCorners[cornersFromEdge[i][0]];
            Point p2 = cubeCorners[cornersFromEdge[i][1]];
            if (cubeMask & (1 << i))
              interpolatedEdges[i] = interpolate_vertices(p1, p2, isolevel, false);
          }

          const int *edges = triTable[cubePattern];
          int i = 0;
          while (edges[i] != -1) {
            triangles.push_back(interpolatedEdges[edges[i]]);
            triangles.push_back(interpolatedEdges[edges[i + 1]]);
            triangles.push_back(interpolatedEdges[edges[i + 2]]);
            i += 3;
          }
        }
      }
    }
  }

  // Setting Up VBO and VAO
  std::vector<glm::vec3> normals = calcNormals(triangles);
  std::vector<float> vertices;
  vertices.reserve(triangles.size() * 6);

  size_t i = 0;
  for (const glm::vec3 vert : triangles) {
    vertices.push_back(vert.x);
    vertices.push_back(vert.y);
    vertices.push_back(vert.z);

    vertices.push_back(normals[i].x);
    vertices.push_back(normals[i].y);
    vertices.push_back(normals[i].z);
    i += 1;
  }

  // std::cout << vertices.size() << std::endl;

  glGenBuffers(1, &VBO);
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
               vertices.data(), GL_STATIC_DRAW);

  // position
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  // normal
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // unbind VAO
  glBindVertexArray(0);
}

void Chunk::render(unsigned int shaderProgram) {
  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(this->chunkPos.x, this->chunkPos.y, this->chunkPos.z));
  // float angle = 20.0f * i;
  // model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f,
  // 0.5f));
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE,
                     &model[0][0]);

  glBindVertexArray(this->VAO);

  glDrawArrays(GL_TRIANGLES, 0, triangles.size());

  glBindVertexArray(0);
}
