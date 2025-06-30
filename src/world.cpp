#include "world.h"
#include "chunk.h"
#include <cmath>
#include <iostream>
#include <vector>

World::World() {}

void World::update(const glm::vec3& playerPos) {
    glm::ivec2 playerChunkCoord(floor(playerPos.x / Chunk::CHUNK_WIDTH), floor(playerPos.z / Chunk::CHUNK_DEPTH));

    // check if chunks need to be unloaded
    std::vector<glm::ivec2> unloadChunks;
    for (auto const& [coord, chunk] : activeChunks) {
        int dist_x = abs(coord.x - playerPos.x);
        int dist_z = abs(coord.y - playerPos.y); // .y == 2nd element of ivec2
        if (dist_x > viewDistance || dist_z > viewDistance) {
            unloadChunks.push_back(coord);
        }
    }

    // call destructor of unloading chunks
    for (const auto& coord : unloadChunks) {
        activeChunks.erase(coord);
    }

    // all chunks within range of player's current chunk get loaded if not already
    for (int x = -viewDistance; x <= viewDistance; x++) {
        for (int z = -viewDistance; z <= viewDistance; z++) {
            glm::ivec2 chunkCoord = playerChunkCoord + glm::ivec2(x, z);
            if (activeChunks.find(chunkCoord) == activeChunks.end()) {
                glm::vec3 chunkWorldPos(chunkCoord.x * Chunk::CHUNK_WIDTH, 0.0f, chunkCoord.y * Chunk::CHUNK_DEPTH);

                activeChunks.emplace(chunkCoord, chunkWorldPos);
            };
        }
    }
}

void World::render(unsigned int shaderProgram) {
    for (auto& [coord, chunk] : activeChunks) {
        chunk.render(shaderProgram);
    }
}
