#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

enum class CameraMode {
    ThirdPerson,
    FirstPerson,
    Spectator
};

class Camera {
public:
    void toggleMode();
    void setMode(CameraMode mode);
    glm::mat4 viewMatrix(const glm::vec3& robotPosition, float robotYaw) const;

    CameraMode mode() const;

private:
    CameraMode currentMode = CameraMode::ThirdPerson;
};
