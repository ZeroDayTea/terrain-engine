#pragma once
#include <glm/glm.hpp>

struct Frustum {
    glm::vec4 planes[6];
};

inline Frustum make_frustum(const glm::mat4& VP) {
    Frustum f;

    // left
    // m03 + m00, m13 + m10, m23 + m20, m33 + m30
    f.planes[0] = glm::vec4(VP[0][3] + VP[0][0],
                            VP[1][3] + VP[1][0],
                            VP[2][3] + VP[2][0],
                            VP[3][3] + VP[3][0]);

    // right
    // m03 - m00, m13 - m10, m23 - m20, m33 - m30
    f.planes[1] = glm::vec4(VP[0][3] - VP[0][0],
                            VP[1][3] - VP[1][0],
                            VP[2][3] - VP[2][0],
                            VP[3][3] - VP[3][0]);

    // bottom
    // m03 + m01, m13 + m11, m23 + m21, m33 + m31
    f.planes[2] = glm::vec4(VP[0][3] + VP[0][1],
                            VP[1][3] + VP[1][1],
                            VP[2][3] + VP[2][1],
                            VP[3][3] + VP[3][1]);

    // top
    // m03 - m01, m13 - m11, m23 - m21, m33 - m31
    f.planes[3] = glm::vec4(VP[0][3] - VP[0][1],
                            VP[1][3] - VP[1][1],
                            VP[2][3] - VP[2][1],
                            VP[3][3] - VP[3][1]);

    // near
    // m03 + m02, m13 + m12, m23 + m22, m33 + m32
    f.planes[4] = glm::vec4(VP[0][3] + VP[0][2],
                            VP[1][3] + VP[1][2],
                            VP[2][3] + VP[2][2],
                            VP[3][3] + VP[3][2]);

    // far
    // m03 - m02, m13 - m12, m23 - m22, m33 - m32
    f.planes[5] = glm::vec4(VP[0][3] - VP[0][2],
                            VP[1][3] - VP[1][2],
                            VP[2][3] - VP[2][2],
                            VP[3][3] - VP[3][2]);

    for (int i = 0; i < 6; ++i) {
        float len = glm::length(glm::vec3(f.planes[i]));
        if (len > 0.0f) {
            f.planes[i] /= len;
        }
    }

    return f;
}

inline bool aabb_outside_plane(const glm::vec3& bmin, const glm::vec3& bmax, const glm::vec4& p) {
    const glm::vec3 n = glm::vec3(p);
    glm::vec3 c = 0.5f * (bmin + bmax);
    glm::vec3 e = 0.5f * (bmax - bmin);
    float s = glm::dot(n, c) + p.w;
    float r = glm::dot(glm::abs(n), e);

    return (s + r) < 0.0f;
}

inline bool aabb_in_frustum(const glm::vec3& bmin, const glm::vec3& bmax, const Frustum& f) {
    for(int i = 0; i < 6; ++i) {
        if (aabb_outside_plane(bmin, bmax, f.planes[i])) return false;
    }
    return true;
}
