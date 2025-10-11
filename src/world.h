#pragma once

#include <map>
#include <unordered_set>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "frustum.h"
#include "chunk.h"
#include <job_queues.h>
#include <worker_types.h>

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
    World(unsigned int densityProgram, unsigned int mcCountProgram, unsigned int mcEmitProgram, BlockingQueue<GenJob>* jobIn, SPSCQueue<GenResult>* jobOut);

    // check player position and load/unload chunks
    void update(const glm::vec3& playerPos);

    void collectFinished();

    // render all loaded chunks
    void render(unsigned int shaderProgram, const Frustum& frustum, GLint uModelLoc);

private:
    // currently loaded chunks
    std::map<glm::ivec3, Chunk, Vec3Compare> activeChunks;
    std::unordered_set<long long> requestedKeys;

    int viewDistance = 6;

    unsigned int densityProgram;
    unsigned int mcCountProgram;
    unsigned int mcEmitProgram;

    BlockingQueue<GenJob>* genIn = nullptr;
    SPSCQueue<GenResult>* genOut = nullptr;

    // FIX THIS: check for better alternative
    static long long key64(int x,int y,int z){ return ( ( (long long)x & 0x1FFFFF)<<42 ) | ( ((long long)y & 0x1FFFFF)<<21 ) | ((long long)z & 0x1FFFFF); }
};
