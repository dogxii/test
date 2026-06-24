#include "world/Robot.h"

#include <algorithm>
#include <cmath>

#include <GLFW/glfw3.h>

#include <glm/geometric.hpp>

#include "math/MathUtils.h"
#include "render/ShapeRenderer.h"

namespace {
constexpr float PI = 3.1415926535f;
constexpr float GroundY = -1.42f;
constexpr float CharacterScale = 0.2f;
constexpr float OriginalFeetY = -1.43f;

void drawRobotShadow(float x, float y, float z, float scale) {
    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.04f, 0.1f, 0.08f, 0.22f);

    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(0.42f * scale, 1.0f, 0.32f * scale);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, 0.0f, 0.0f);
    for (int i = 0; i <= 20; ++i) {
        const float angle = static_cast<float>(i) / 20.0f * PI * 2.0f;
        glVertex3f(std::cos(angle), 0.0f, std::sin(angle));
    }
    glEnd();
    glPopMatrix();

    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glEnable(GL_FOG);
    glEnable(GL_LIGHTING);
}
}

void Robot::update(float deltaTime, const InputState& input, CameraMode cameraMode) {
    glm::vec3 movement {0.0f};

    if (cameraMode == CameraMode::FirstPerson || cameraMode == CameraMode::Spectator) {
        if (input.left()) {
            currentYaw += turnSpeed * deltaTime;
        }

        if (input.right()) {
            currentYaw -= turnSpeed * deltaTime;
        }

        const glm::vec3 forward = forwardFromYaw(currentYaw);

        if (input.forward()) {
            movement += forward;
        }

        if (input.backward()) {
            movement += forward * -1.0f;
        }

        if (cameraMode == CameraMode::Spectator) {
            if (input.ascend()) {
                currentPosition.y += moveSpeed * deltaTime;
            }

            if (input.descend()) {
                currentPosition.y -= moveSpeed * deltaTime;
            }

            currentPosition.y = std::clamp(currentPosition.y, 0.0f, 10.0f);
        }
    } else {
        if (input.forward()) {
            movement += glm::vec3(0.0f, 0.0f, -1.0f);
        }

        if (input.backward()) {
            movement += glm::vec3(0.0f, 0.0f, 1.0f);
        }

        if (input.left()) {
            movement += glm::vec3(-1.0f, 0.0f, 0.0f);
        }

        if (input.right()) {
            movement += glm::vec3(1.0f, 0.0f, 0.0f);
        }
    }

    const bool moving = glm::length(movement) > 0.0f;

    if (moving) {
        const glm::vec3 direction = glm::normalize(movement);
        currentPosition += direction * moveSpeed * deltaTime;
        const float movementLimit = cameraMode == CameraMode::Spectator ? 24.0f : 20.4f;
        currentPosition.x = std::clamp(currentPosition.x, -movementLimit, movementLimit);
        currentPosition.z = std::clamp(currentPosition.z, -movementLimit, movementLimit);
        if (cameraMode == CameraMode::ThirdPerson) {
            currentYaw = glm::degrees(std::atan2(-direction.x, -direction.z));
        }
        walkTime += deltaTime * 8.0f;
    } else {
        walkTime *= std::max(0.0f, 1.0f - deltaTime * 6.0f);
    }

    attackTime = std::max(0.0f, attackTime - deltaTime);
}

void Robot::startAttack() {
    attackTime = 0.12f;
}

void Robot::render(bool hideBody) const {
    if (hideBody) {
        return;
    }

    const float attackSwing = attackTime > 0.0f ? -12.0f : 0.0f;
    const float swing = std::sin(walkTime) * 24.0f;

    drawRobotShadow(currentPosition.x, GroundY + currentPosition.y, currentPosition.z, CharacterScale);

    glPushMatrix();
    glTranslatef(
        currentPosition.x,
        currentPosition.y + GroundY - OriginalFeetY * CharacterScale,
        currentPosition.z
    );
    glRotatef(currentYaw, 0.0f, 1.0f, 0.0f);
    glScalef(CharacterScale, CharacterScale, CharacterScale);

    ShapeRenderer::setMaterial({0.1f, 0.62f, 1.0f});
    drawPart(0.0f, 0.21f, 0.0f, 0.58f, 0.58f, 0.5f);

    ShapeRenderer::setMaterial({0.96f, 0.98f, 0.9f}, 6.0f);
    drawPart(-0.13f, 0.25f, -0.26f, 0.12f, 0.1f, 0.035f);
    drawPart(0.13f, 0.25f, -0.26f, 0.12f, 0.1f, 0.035f);

    ShapeRenderer::setMaterial({0.04f, 0.06f, 0.08f}, 4.0f);
    drawPart(-0.1f, 0.24f, -0.285f, 0.045f, 0.045f, 0.025f);
    drawPart(0.1f, 0.24f, -0.285f, 0.045f, 0.045f, 0.025f);

    glPushMatrix();
    glTranslatef(-0.13f, 0.36f, -0.285f);
    glRotatef(-12.0f, 0.0f, 0.0f, 1.0f);
    drawPart(0.0f, 0.0f, 0.0f, 0.17f, 0.035f, 0.025f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.13f, 0.36f, -0.285f);
    glRotatef(12.0f, 0.0f, 0.0f, 1.0f);
    drawPart(0.0f, 0.0f, 0.0f, 0.17f, 0.035f, 0.025f);
    glPopMatrix();

    ShapeRenderer::setMaterial({0.08f, 0.34f, 0.7f});
    drawPart(0.0f, -0.49f, 0.0f, 0.62f, 0.9f, 0.36f);

    ShapeRenderer::setMaterial({0.1f, 0.62f, 1.0f});
    glPushMatrix();
    glTranslatef(-0.46f, -0.28f, 0.0f);
    glRotatef(swing, 1.0f, 0.0f, 0.0f);
    drawPart(0.0f, -0.25f, 0.0f, 0.22f, 0.72f, 0.26f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.46f, -0.28f, 0.0f);
    glRotatef(-swing + attackSwing, 1.0f, 0.0f, 0.0f);
    drawPart(0.0f, -0.25f, 0.0f, 0.22f, 0.72f, 0.26f);

    ShapeRenderer::setMaterial({0.03f, 0.035f, 0.04f}, 6.0f);
    drawPart(0.0f, -0.58f, -0.28f, 0.14f, 0.12f, 0.46f);
    drawPart(0.0f, -0.47f, -0.48f, 0.08f, 0.08f, 0.18f);
    glPopMatrix();

    ShapeRenderer::setMaterial({0.06f, 0.28f, 0.62f});
    glPushMatrix();
    glTranslatef(-0.18f, -0.93f, 0.0f);
    glRotatef(-swing, 1.0f, 0.0f, 0.0f);
    drawPart(0.0f, -0.25f, 0.0f, 0.22f, 0.5f, 0.26f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.18f, -0.93f, 0.0f);
    glRotatef(swing, 1.0f, 0.0f, 0.0f);
    drawPart(0.0f, -0.25f, 0.0f, 0.22f, 0.5f, 0.26f);
    glPopMatrix();

    glPopMatrix();
}

void Robot::setPosition(const glm::vec3& position) {
    currentPosition = position;
}

const glm::vec3& Robot::position() const {
    return currentPosition;
}

float Robot::yaw() const {
    return currentYaw;
}

float Robot::collisionRadius() const {
    return 0.08f;
}

void Robot::drawPart(float x, float y, float z, float sx, float sy, float sz) const {
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(sx, sy, sz);
    ShapeRenderer::cube();
    glPopMatrix();
}
