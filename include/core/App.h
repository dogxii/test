#pragma once

#include "core/Camera.h"
#include "core/InputState.h"
#include "game/GameState.h"
#include "world/Robot.h"
#include "world/Scene.h"

#include <string>

struct GLFWwindow;

enum class AppScreen {
    Start,
    Playing,
    Paused
};

class App {
public:
    bool init(int width, int height, const char* title);
    void run();
    void shutdown();

private:
    GLFWwindow* window = nullptr;
    int windowWidth = 1280;
    int windowHeight = 720;
    double previousTime = 0.0;
    InputState input;
    Camera camera;
    GameState gameState;
    Robot robot;
    Scene scene;
    std::string currentTitle;
    AppScreen screen = AppScreen::Start;

    void resize(int width, int height);
    void update(float deltaTime);
    void render();
    void setKey(int key, int action);
    void startGame();
    void restartGame();
    void setupProjection() const;
    void setupLights() const;
    void renderSky() const;
    void renderHud() const;
    void renderStartScreen() const;
    void renderPauseScreen() const;
    void updateWindowTitle();

    static void handleResize(GLFWwindow* window, int width, int height);
    static void handleKey(GLFWwindow* window, int key, int scancode, int action, int mods);
};
