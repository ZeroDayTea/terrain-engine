#pragma once

#include "chunk.h"
#include <map>
#include <glm/glm.hpp>

struct Vec3Compare {
    bool operator()(const glm::ivec3& a, const glm::ivec3& b) const {
        if (a.x < b.x) return true;
        if (a.x > b.x) return false;
        if (a.y < b.y) return true;
        if (a.y > b.y) return false;
        if (a.z < b.z) return true;
        if (a.z > b.z) return false;
        return false;
    }
};

class World {
public:
    World();

    // check player position and load/unload chunks
    void update(const glm::vec3& playerPos);

    // render all loaded chunks
    void render(unsigned int shaderProgram);

private:
    // currently loaded chunks
    std::map<glm::ivec3, Chunk, Vec3Compare> activeChunks;

    int viewDistance = 4;
};
