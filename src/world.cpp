#include "world.h"
#include "chunk.h"
#include <cmath>
#include <iostream>
#include <vector>

World::World(unsigned int densityProg, unsigned int marchingProg) : densityProgram(densityProg), marchingProgram(marchingProg) {}

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
        for (int y = -viewDistance; y <= viewDistance; y++) {
            for (int z = -viewDistance; z <= viewDistance; z++) {
                glm::ivec3 chunkCoord = playerChunkCoord + glm::ivec3(x, y, z);
                if (activeChunks.find(chunkCoord) == activeChunks.end()) {
                    glm::vec3 chunkWorldPos(
                        chunkCoord.x * Chunk::CHUNK_WIDTH,
                        chunkCoord.y * Chunk::CHUNK_HEIGHT,
                        chunkCoord.z * Chunk::CHUNK_DEPTH
                    );

                    activeChunks.try_emplace(chunkCoord, chunkWorldPos, this->densityProgram, this->marchingProgram);
                };
            }
        }
    }
}

void World::render(unsigned int shaderProgram) {
    for (auto& [coord, chunk] : activeChunks) {
        chunk.render(shaderProgram);
    }
}
