#pragma once

#include <atomic>
#include <thread>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

template<typename T> class BlockingQueue;
template<typename T> class SPSCQueue;
struct GenJob;
struct GenResult;

class ChunkWorker {
public:
  ChunkWorker(GLFWwindow* worker,
              BlockingQueue<GenJob>* in,
              SPSCQueue<GenResult>* out,
              GLuint density,
              GLuint mcCount,
              GLuint mcEmit,
              GLuint triTableSSBO,
              GLuint edgeTableSSBO);

  void start();
  void shutdown();

private:
  void run();

  GLFWwindow* worker_window = nullptr;
  BlockingQueue<GenJob>* inQ = nullptr;
  SPSCQueue<GenResult>* outQ = nullptr;
  std::thread th;
  std::atomic<bool> stop{false};

  GLuint densityProg = 0;
  GLuint mcCountProg = 0;
  GLuint mcEmitProg = 0;
  GLuint triSSBO = 0;
  GLuint edgeSSBO = 0;

  const int terrainMode = 1;
};
