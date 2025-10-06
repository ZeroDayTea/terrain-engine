#include "world.h"
#include "chunk.h"
#include "frustum.h"
#include <cmath>
#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

World::World(unsigned int densityProg, unsigned int mcCountProg, unsigned int mcEmitProg) : densityProgram(densityProg), mcCountProgram(mcCountProg), mcEmitProgram(mcEmitProg) {}

void World::update(const glm::vec3& playerPos) {
    glm::ivec3 playerChunkCoord(
        floor(playerPos.x / Chunk::CHUNK_WIDTH),
        floor(playerPos.y / Chunk::CHUNK_HEIGHT),
        floor(playerPos.z / Chunk::CHUNK_DEPTH)
    );

    // check if chunks need to be unloaded
    std::vector<glm::ivec3> unloadChunks;
    for (auto const& [coord, chunk] : activeChunks) {
        int dist_x = abs(coord.x - playerChunkCoord.x);
        int dist_y = abs(coord.y - playerChunkCoord.y);
        int dist_z = abs(coord.z - playerChunkCoord.z);
        if (dist_x > viewDistance || dist_y > viewDistance || dist_z > viewDistance) {
            unloadChunks.push_back(coord);
        }
    }

    // call destructor of unloading chunks
    for (const auto& coord : unloadChunks) {
        activeChunks.erase(coord);
    }

    // all chunks within range of player's current chunk get loaded if not already
    for (int x = -viewDistance; x <= viewDistance; x++) {
        for (int y = -4; y <= 4; y++) {
            for (int z = -viewDistance; z <= viewDistance; z++) {
                glm::ivec3 chunkCoord = playerChunkCoord + glm::ivec3(x, y, z);
                if (activeChunks.find(chunkCoord) == activeChunks.end()) {
                    glm::vec3 chunkWorldPos(
                        chunkCoord.x * Chunk::CHUNK_WIDTH,
                        chunkCoord.y * Chunk::CHUNK_HEIGHT,
                        chunkCoord.z * Chunk::CHUNK_DEPTH
                    );

                    activeChunks.try_emplace(chunkCoord, chunkWorldPos, this->densityProgram, this->mcCountProgram, this->mcEmitProgram);
                };
            }
        }
    }
}

void World::render(unsigned int shaderProgram, const Frustum& frustum, GLint uModelLoc) {
    for (auto& [coord, chunk] : activeChunks) {
        glm::vec3 bmin = chunk.chunkPos;
        glm::vec3 bmax = chunk.chunkPos + glm::vec3(Chunk::CHUNK_WIDTH, Chunk::CHUNK_HEIGHT, Chunk::CHUNK_DEPTH);
        if (!aabb_in_frustum(bmin, bmax, frustum)) continue;

        glm::mat4 model = glm::translate(glm::mat4(1.0f), chunk.chunkPos);
        glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, &model[0][0]);
        chunk.renderRaw();
    }
}
