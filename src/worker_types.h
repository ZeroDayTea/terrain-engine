#pragma once

#include <glm/glm.hpp>
#include <glad/glad.h>

struct ChunkKey {
    int x, y, z;
};

inline bool operator==(const ChunkKey &a, const ChunkKey &b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

struct GenJob {
    ChunkKey key;
    glm::vec3 worldPos;
};

struct GenResult {
    ChunkKey key;
    glm::vec3 worldPos;
    GLuint vertexSSBO = 0; // (3*uint32 per vertex)
    GLuint indirect = 0;
    GLuint counterSSBO = 0;
    GLuint densitySSBO = 0;
    GLuint offsetsSSBO = 0;
    GLsync fence = 0;
    GLuint totalVertices = 0;
};
