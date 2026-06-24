#pragma once

#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>
#include <glm/vec3.hpp>

inline glm::vec3 forwardFromYaw(float yawDegrees) {
    const float radians = glm::radians(yawDegrees);
    return glm::normalize(glm::vec3(-std::sin(radians), 0.0f, -std::cos(radians)));
}
