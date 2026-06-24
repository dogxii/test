#pragma once

class InputState {
public:
    void setKey(int key, bool pressed);
    void clear();

    bool forward() const;
    bool backward() const;
    bool left() const;
    bool right() const;
    bool ascend() const;
    bool descend() const;
    bool consumeCameraToggle();
    bool consumeAttack();

private:
    bool forwardPressed = false;
    bool backwardPressed = false;
    bool leftPressed = false;
    bool rightPressed = false;
    bool ascendPressed = false;
    bool descendPressed = false;
    bool cameraToggleRequested = false;
    bool attackRequested = false;
};
