#include "chunk.h"

#include "marching_cubes.h"

#define DB_PERLIN_IMPL
#include "db_perlin.hpp"

#include <cmath>
#include <iostream>
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
                               float isolevel) {
  // if (std::abs(p1.value - p2.value) < 0.00001f) {
  //   return p1.pos;
  // }
  // float t = (isolevel - p1.value) / (p2.value - p1.value);
  // return p1.pos + t * (p2.pos - p1.pos);
  return (1.0f / 2.0f) * (p1.pos + p2.pos);
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
    glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f),
    glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f),
    glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 1.0f)};

const float Chunk::cubeSize = 1.0f;

const double Chunk::isolevel = 0.0f;

Chunk::Chunk(glm::vec3 chunkPosition) : chunkPos(chunkPosition) {
  generateMesh();
}

Chunk::~Chunk() {
  // cleanup
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
}

void Chunk::generateMesh() {
  for (int x = 0; x < CHUNK_DEPTH; ++x) {
    for (int y = 0; y < CHUNK_WIDTH; ++y) {
      for (int z = 0; z < CHUNK_HEIGHT; ++z) {
        glm::vec3 worldPos = glm::vec3(x, y, z) * cubeSize + this->chunkPos;
        glm::vec3 samplePos =
            glm::vec3(worldPos.x * 0.5f, worldPos.y * 0.25f, worldPos.z * 0.5f);
        // double noise3D = 0.0f;
        // double frequency = 0.0035f;
        // double amplitude = 1.0f;
        // double lacunarity = 2.0f;
        // double persistence = 0.45f;
        // // octaves
        // for (int j = 0; j < 6; j++) {
        //   double n =
        //       db::perlin(samplePos.x * frequency, samplePos.z * frequency,
        //                  samplePos.y * frequency);
        //   noise3D += n * amplitude;
        //   amplitude *= persistence;
        //   frequency *= lacunarity;
        // }
        // double floorOffset = 1.0f;
        // double noiseWeight = 5.0f;
        // double finalVal = (-worldPos.y + floorOffset) + noise3D *
        // noiseWeight; this->points[x][y][z] = finalVal; std::cout << finalVal
        // << std::endl;

        this->points[x][y][z] =
            db::perlin(double(samplePos.x) * 0.05, double(samplePos.y) * 0.05,
                       double(samplePos.z) * 0.05);
      }
    }
  }

  for (int x = 0; x < CHUNK_DEPTH - 1; ++x) {
    for (int y = 0; y < CHUNK_WIDTH - 1; ++y) {
      for (int z = 0; z < CHUNK_HEIGHT - 1; ++z) {

        Point cubeCorners[8];
        glm::vec3 cubePos = glm::vec3(x, y, z);

        for (int i = 0; i < 8; i++) {
          glm::vec3 offset = cornerOffsets[i];
          cubeCorners[i].pos = (cubePos + offset) * cubeSize;
          // std::cout << this->points[x][y][z] << std::endl;
          cubeCorners[i].value =
              this->points[static_cast<int>(x + offset.x)][static_cast<int>(
                  y + offset.y)][static_cast<int>(z + offset.z)];
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
              interpolatedEdges[i] = interpolate_vertices(p1, p2, isolevel);
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
  for (const glm::vec3 vert : triangles) {
    vertices.push_back(vert.x);
    vertices.push_back(vert.y);
    vertices.push_back(vert.z);
  }

  std::cout << vertices.size() << std::endl;

  glGenBuffers(1, &VBO);
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
               vertices.data(), GL_STATIC_DRAW);

  // position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // render triangle
  glBindVertexArray(VAO);
}

void Chunk::render(unsigned int shaderProgram) {
  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, this->chunkPos);
  // float angle = 20.0f * i;
  // model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f,
  // 0.5f));
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE,
                     &model[0][0]);

  glDrawArrays(GL_TRIANGLES, 0, triangles.size());
}
