#pragma once
#include <glm/glm.hpp>

struct Frustum {
    glm::vec4 planes[6];
};

Frustum make_frustum(const glm::mat4& VP);
bool aabb_in_frustum(const glm::vec3& bmin, const glm::vec3& bmax, const Frustum& f);

