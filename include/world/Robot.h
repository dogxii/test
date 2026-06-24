#pragma once

#include "core/Camera.h"
#include "core/InputState.h"

#include <glm/vec3.hpp>

class Robot {
public:
    void update(float deltaTime, const InputState& input, CameraMode cameraMode);
    void startAttack();
    void render(bool hideBody) const;
    void setPosition(const glm::vec3& position);

    const glm::vec3& position() const;
    float yaw() const;
    float collisionRadius() const;

private:
    glm::vec3 currentPosition {0.0f, 0.0f, 5.0f};
    float currentYaw = 0.0f;
    float walkTime = 0.0f;
    float attackTime = 0.0f;
    float moveSpeed = 5.0f;
    float turnSpeed = 100.0f;

    void drawPart(float x, float y, float z, float sx, float sy, float sz) const;
};
