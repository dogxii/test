#include "core/InputState.h"

#include <GLFW/glfw3.h>

void InputState::setKey(int key, bool pressed) {
    switch (key) {
        case GLFW_KEY_W:
            forwardPressed = pressed;
            break;
        case GLFW_KEY_S:
            backwardPressed = pressed;
            break;
        case GLFW_KEY_A:
            leftPressed = pressed;
            break;
        case GLFW_KEY_D:
            rightPressed = pressed;
            break;
        case GLFW_KEY_LEFT_SHIFT:
        case GLFW_KEY_RIGHT_SHIFT:
            descendPressed = pressed;
            break;
        case GLFW_KEY_C:
            if (pressed) {
                cameraToggleRequested = true;
            }
            break;
        case GLFW_KEY_SPACE:
            ascendPressed = pressed;
            if (pressed) {
                attackRequested = true;
            }
            break;
        default:
            break;
    }
}

void InputState::clear() {
    forwardPressed = false;
    backwardPressed = false;
    leftPressed = false;
    rightPressed = false;
    ascendPressed = false;
    descendPressed = false;
    cameraToggleRequested = false;
    attackRequested = false;
}

bool InputState::forward() const {
    return forwardPressed;
}

bool InputState::backward() const {
    return backwardPressed;
}

bool InputState::left() const {
    return leftPressed;
}

bool InputState::right() const {
    return rightPressed;
}

bool InputState::ascend() const {
    return ascendPressed;
}

bool InputState::descend() const {
    return descendPressed;
}

bool InputState::consumeCameraToggle() {
    if (!cameraToggleRequested) {
        return false;
    }

    cameraToggleRequested = false;
    return true;
}

bool InputState::consumeAttack() {
    if (!attackRequested) {
        return false;
    }

    attackRequested = false;
    return true;
}
