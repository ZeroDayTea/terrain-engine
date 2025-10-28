#pragma once

namespace Config {
    constexpr unsigned int SCREEN_WIDTH = 1920;
    constexpr unsigned int SCREEN_HEIGHT = 1080;

    constexpr int CHUNK_WIDTH = 64;
    constexpr int CHUNK_HEIGHT = 64;
    constexpr int CHUNK_DEPTH = 64;

    constexpr int VIEW_DISTANCE = 6;
    constexpr float ISOLEVEL = 0.0f;

    constexpr float RENDER_DISTANCE = 2000.0f;
    constexpr float NEAR_PLANE = 0.1f;
    constexpr float FOV = 45.0f;

    constexpr float CAMERA_SPEED = 20.0f;
    constexpr float MOUSE_SENSITIVITY = 2.0f;

    const char* const SHADER_DIR = "../shaders/";
}
