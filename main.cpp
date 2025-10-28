#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "camera.h"
#include "init.h"
#include "config.h"

#include "load_shaders.h"
#include "frustum.h"
#include "gl_shared.h"

#include "chunk.h"
#include "world.h"
#include "chunk_worker.h"
#include "marching_cubes.h"

#include "job_queues.h"
#include "worker_types.h"


float deltaTime = 0.0f;
float lastFrame = 0.0f;

void processInput(GLFWwindow* window, Camera& camera, float dt);
GLuint loadTerrainTexture(const char* path);

int main() {
    GLFWwindow* window = glfw_initialization();
    if (!window) {
        std::cerr << "Failed to create main window\n";
        return -1;
    }
    GLFWwindow* worker_window = create_shared_context(window);
    if (!worker_window) {
        std::cerr << "Failed to create shared worker context window\n";
        return -1;
    }

    Camera& camera = get_camera();

    // build marching cubes lookup tables once
    GLuint g_triSSBO = 0, g_edgeSSBO = 0;
    glGenBuffers(1, &g_triSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_triSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(triTable), &triTable[0], GL_STATIC_DRAW);

    glGenBuffers(1, &g_edgeSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_edgeSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(edgeTable), &edgeTable[0], GL_STATIC_DRAW);

    // compile shaders
    unsigned int shaderProgram = generate_shader_program();
    unsigned int densityComputeProgram = generate_compute_program("../shaders/snoise.comp", "../shaders/density.comp");
    unsigned int mcCountComputeProgram = generate_compute_program("../shaders/mc_count.comp");
    unsigned int mcEmitComputeProgram = generate_compute_program("../shaders/mc_emit.comp");
    auto U = get_locations(shaderProgram);

    // load noise texture for terrain coloring
    GLuint noiseTex = loadTerrainTexture("../shaders/terrainnoise.png");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, noiseTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    // TODO
    GLfloat aniso = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, std::min(aniso, 8.0f));

    // setup worker for multi-threaded chunk jobs
    BlockingQueue<GenJob> genIn;
    SPSCQueue<GenResult> genOut;
    ChunkWorker worker(worker_window, &genIn, &genOut, densityComputeProgram, mcCountComputeProgram, mcEmitComputeProgram, g_triSSBO, g_edgeSSBO);
    worker.start();

    {
        // create world
        World world(densityComputeProgram, mcCountComputeProgram, mcEmitComputeProgram, &genIn, &genOut);

        // sun-like lighting
        glm::vec3 lightColor(1.0f, 0.95f, 0.9f);
        glm::vec3 lightPos(24.0f, 50.0f, 24.0f);

        // render loop
        while (!glfwWindowShouldClose(window)) {
            float currentFrame = static_cast<float>(glfwGetTime());
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            processInput(window, camera, deltaTime);
            world.update(camera.Position);
            world.collectFinished();

            // sky blue color
            glClearColor(0.5f, 0.7f, 0.9f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glUseProgram(shaderProgram);

            glm::mat4 view = camera.GetViewMatrix();
            glm::mat4 projection = glm::perspective(
                glm::radians(camera.Zoom),
                (float)Config::SCREEN_WIDTH / (float)Config::SCREEN_HEIGHT,
                Config::NEAR_PLANE,
                Config::RENDER_DISTANCE); // setting near-plane and far-plane

            glUniformMatrix4fv(U.uView, 1, GL_FALSE, &view[0][0]);
            glUniformMatrix4fv(U.uProj, 1, GL_FALSE, &projection[0][0]);
            glUniform3fv(U.uLightColor, 1, &lightColor[0]);
            glUniform3fv(U.uLightPos, 1, &lightPos[0]);
            glUniform3fv(U.uViewPos, 1, &camera.Position[0]);

            Frustum fr = make_frustum(projection * view);
            world.render(shaderProgram, fr, U.uModel);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    worker.shutdown();
    glDeleteProgram(shaderProgram);
    glDeleteProgram(densityComputeProgram);
    glDeleteProgram(mcCountComputeProgram);
    glDeleteProgram(mcEmitComputeProgram);
    glDeleteBuffers(1, &g_triSSBO);
    glDeleteBuffers(1, &g_edgeSSBO);
    glDeleteTextures(1, &noiseTex);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window, Camera& camera, float dt) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) 
        camera.ProcessKeyboard(FORWARD, dt);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, dt);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, dt);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, dt);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, dt);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, dt);
}

GLuint loadTerrainTexture(const char* path) {
    int w=0, h=0, comp=0;
    stbi_uc* data = stbi_load(path, &w, &h, &comp, 1);
    if (!data) {
        std::cerr << "Failed to load terrain color noise texture\n";
        return 0;
    }

    GLuint tex=0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    GLenum fmt = (comp == 1) ? GL_RED : (comp == 3) ? GL_RGB : GL_RGBA;

    glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return tex;
}
