#pragma once

#include <atomic>
#include <thread>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "job_queues.h"
#include "worker_types.h"
#include "chunk.h"
#include "marching_cubes.h"

class ChunkWorker {
    GLFWwindow* worker_window = nullptr;
    BlockingQueue<GenJob>* inQ = nullptr;
    SPSCQueue<GenResult>* outQ = nullptr;
    std::thread th;
    std::atomic<bool> stop{false};

    GLuint densityProg = 0, mcCountProg = 0, mcEmitProg = 0;
    GLuint triSSBO = 0, edgeSSBO = 0;

public:
    ChunkWorker(GLFWwindow* worker, BlockingQueue<GenJob>* in, SPSCQueue<GenResult>* out,
                GLuint density, GLuint mcCount, GLuint mcEmit,
                GLuint triTableSSBO, GLuint edgeTableSSBO)
        : worker_window(worker), inQ(in), outQ(out),
          densityProg(density), mcCountProg(mcCount), mcEmitProg(mcEmit),
          triSSBO(triTableSSBO), edgeSSBO(edgeTableSSBO) {}

    void start() {
        th = std::thread([this]{ run(); });
    }

    void shutdown() {
        stop = true;
        if (inQ) {
            inQ->close();
        }
        if (th.joinable()) {
            th.join();
        }
    }

private:
    void run() {
        glfwMakeContextCurrent(worker_window);
        gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

        while(!stop){
            auto jobOpt = inQ->pop();
            if (!jobOpt.has_value()) break;
            GenJob job = *jobOpt;

            GenResult res{}; res.key = job.key; res.worldPos = job.worldPos;

            // allocate buffers
            glGenBuffers(1, &res.densitySSBO);
            glGenBuffers(1, &res.offsetsSSBO);
            glGenBuffers(1, &res.counterSSBO);
            glGenBuffers(1, &res.vertexSSBO);
            glGenBuffers(1, &res.indirect);

            const size_t NUM_POINTS = (Chunk::CHUNK_WIDTH+1)*(Chunk::CHUNK_HEIGHT+1)*(Chunk::CHUNK_DEPTH+1);
            const size_t NUM_VOXELS = (Chunk::CHUNK_WIDTH)*(Chunk::CHUNK_HEIGHT)*(Chunk::CHUNK_DEPTH);

            glBindBuffer(GL_SHADER_STORAGE_BUFFER, res.densitySSBO);
            glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_POINTS*sizeof(float), nullptr, GL_STATIC_DRAW);

            glBindBuffer(GL_SHADER_STORAGE_BUFFER, res.offsetsSSBO);
            glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_VOXELS*sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);

            GLuint zero=0;
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, res.counterSSBO);
            glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint), &zero, GL_DYNAMIC_DRAW);

            // first pass: density
            glUseProgram(densityProg);
            glUniform3fv(glGetUniformLocation(densityProg, "chunkWorldPos"), 1, &res.worldPos[0]);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, res.densitySSBO);
            glDispatchCompute((Chunk::CHUNK_WIDTH+1+7)/8, (Chunk::CHUNK_HEIGHT+1+7)/8, (Chunk::CHUNK_DEPTH+1+7)/8);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

            // second pass: count
            glUseProgram(mcCountProg);
            glUniform1f(glGetUniformLocation(mcCountProg, "isolevel"), 0.0f);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, res.densitySSBO);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, triSSBO);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, edgeSSBO);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, res.counterSSBO);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, res.offsetsSSBO);
            glDispatchCompute(Chunk::CHUNK_WIDTH/8, Chunk::CHUNK_HEIGHT/8, Chunk::CHUNK_DEPTH/8);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

            GLsync countFence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
            glClientWaitSync(countFence, GL_SYNC_FLUSH_COMMANDS_BIT, GLuint64(1e9));
            glDeleteSync(countFence);

            GLuint totalVertices = 0;
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, res.counterSSBO);
            glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint), &totalVertices);
            res.totalVertices = totalVertices;

            if (totalVertices == 0) {
                res.fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
                glFlush();
                outQ->push(std::move(res));
                continue;
            }
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, res.vertexSSBO);
            glBufferData(GL_SHADER_STORAGE_BUFFER, totalVertices * 8u * sizeof(float), nullptr, GL_STATIC_DRAW);

            // third pass: emit (packed uvec3 vertices)
            glUseProgram(mcEmitProg);
            glUniform1f(glGetUniformLocation(mcEmitProg, "isolevel"), 0.0f);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, res.densitySSBO);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, triSSBO);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, edgeSSBO);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, res.vertexSSBO);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, res.offsetsSSBO);
            glDispatchCompute(Chunk::CHUNK_WIDTH/8, Chunk::CHUNK_HEIGHT/8, Chunk::CHUNK_DEPTH/8);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

            // fence and flush
            res.fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
            glFlush();

            outQ->push(std::move(res));
        }

        glfwMakeContextCurrent(nullptr);
    }
};
