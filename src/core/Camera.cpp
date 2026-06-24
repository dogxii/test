#include "core/Camera.h"

#include <glm/ext/matrix_transform.hpp>

#include "math/MathUtils.h"

void Camera::toggleMode() {
    if (currentMode == CameraMode::ThirdPerson) {
        currentMode = CameraMode::FirstPerson;
        return;
    }

    if (currentMode == CameraMode::FirstPerson) {
        currentMode = CameraMode::Spectator;
        return;
    }

    currentMode = CameraMode::ThirdPerson;
}

void Camera::setMode(CameraMode mode) {
    currentMode = mode;
}

glm::mat4 Camera::viewMatrix(const glm::vec3& robotPosition, float robotYaw) const {
    const glm::vec3 forward = forwardFromYaw(robotYaw);
    glm::vec3 cameraPosition;
    glm::vec3 target;

    if (currentMode == CameraMode::FirstPerson || currentMode == CameraMode::Spectator) {
        cameraPosition = robotPosition + glm::vec3(0.0f, -1.13f, 0.0f);
        target = cameraPosition + forward;
    } else {
        cameraPosition = robotPosition + glm::vec3(0.0f, 18.0f, 6.2f);
        target = robotPosition + glm::vec3(0.0f, -1.0f, -0.25f);
    }

    return glm::lookAt(cameraPosition, target, glm::vec3(0.0f, 1.0f, 0.0f));
}

CameraMode Camera::mode() const {
    return currentMode;
}
