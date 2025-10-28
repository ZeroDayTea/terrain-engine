#include "world.h"
#include "chunk.h"
#include "frustum.h"
#include "job_queues.h"
#include "worker_types.h"
#include <cmath>
#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

World::World(unsigned int densityProg, unsigned int mcCountProg, unsigned int mcEmitProg, BlockingQueue<GenJob>* jobIn, SPSCQueue<GenResult>* jobOut) : densityProgram(densityProg), mcCountProgram(mcCountProg), mcEmitProgram(mcEmitProg), genIn(jobIn), genOut(jobOut) {}

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
        requestedKeys.erase(key64(coord.x, coord.y, coord.z));
    }

    // all chunks within range of player's current chunk get loaded if not already
    for (int x = -viewDistance; x <= viewDistance; x++) {
        for (int y = -4; y <= 4; y++) {
            for (int z = -viewDistance; z <= viewDistance; z++) {
                glm::ivec3 chunkCoord = playerChunkCoord + glm::ivec3(x, y, z);
                if (activeChunks.find(chunkCoord) != activeChunks.end()) continue;
                long long k = key64(chunkCoord.x, chunkCoord.y, chunkCoord.z);
                if (requestedKeys.count(k)) continue;

                glm::vec3 chunkWorldPos(
                    chunkCoord.x * Chunk::CHUNK_WIDTH,
                    chunkCoord.y * Chunk::CHUNK_HEIGHT,
                    chunkCoord.z * Chunk::CHUNK_DEPTH
                );

                // activeChunks.try_emplace(chunkCoord, chunkWorldPos, this->densityProgram, this->mcCountProgram, this->mcEmitProgram);
                genIn->push(GenJob{ {chunkCoord.x, chunkCoord.y, chunkCoord.z}, chunkWorldPos });
                requestedKeys.insert(k);
            }
        }
    }
}

void World::collectFinished() {
    GenResult res;
    while (genOut->try_pop(res)) {
        // wait/poll once
        GLenum r = glClientWaitSync(res.fence, 0, 0);
        if (r != GL_ALREADY_SIGNALED && r != GL_CONDITION_SATISFIED) {
            // spin
            glClientWaitSync(res.fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1000000);
        }
        glDeleteSync(res.fence);

        GLuint totalVertices = res.totalVertices;
        if (totalVertices == 0) {
            glDeleteBuffers(1,&res.vertexSSBO);
            glDeleteBuffers(1,&res.indirect);
            glDeleteBuffers(1,&res.counterSSBO);
            glDeleteBuffers(1,&res.densitySSBO);
            glDeleteBuffers(1,&res.offsetsSSBO);
            continue;
        }

        // buffer with known count from second pass
        struct IndirectDraw { GLuint count, instanceCount, first, baseInstance; };
        IndirectDraw cmd{ totalVertices, 1u, 0u, 0u };
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, res.indirect);
        glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(cmd), &cmd, GL_STATIC_DRAW);

        // create VAO
        GLuint vao=0;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, res.vertexSSBO);

        const GLsizei stride = 2 * sizeof(GLuint);

        // location 0: aPos
        glEnableVertexAttribArray(0);
        glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, stride, (void*)0);

        // location 1: aNormal
        glEnableVertexAttribArray(1);
        glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, stride, (void*)(sizeof(GLuint)));

        glBindVertexArray(0);

        glm::ivec3 cc(res.key.x,res.key.y,res.key.z);
        auto [it,inserted] = activeChunks.try_emplace(cc, res.worldPos);
        it->second.adoptPrebuilt(vao, res.vertexSSBO, res.indirect);

        glDeleteBuffers(1,&res.counterSSBO);
        glDeleteBuffers(1,&res.densitySSBO);
        glDeleteBuffers(1,&res.offsetsSSBO);
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
