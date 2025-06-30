#pragma once

#include "chunk.h"
#include <map>
#include <glm/glm.hpp>

struct Vec2Compare {
    bool operator()(const glm::ivec2& a, const glm::ivec2& b) const {
        if (a.x < b.x) return true;
        if (a.x > b.x) return false;
        if (a.y < b.y) return true;
        if (a.y > b.y) return false;
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
    std::map<glm::ivec2, Chunk, Vec2Compare> activeChunks;

    int viewDistance = 4;
};
