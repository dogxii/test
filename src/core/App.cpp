#include "core/App.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>

#include <GLFW/glfw3.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "math/MathUtils.h"
#include "render/UiRenderer.h"

namespace {
const glm::vec3 Panel {0.12f, 0.15f, 0.18f};
const glm::vec3 Blue {0.12f, 0.55f, 0.86f};
const glm::vec3 White {0.94f, 0.96f, 0.98f};
const glm::vec3 Gray {0.65f, 0.7f, 0.74f};

const char* cameraLabel(CameraMode mode) {
    switch (mode) {
        case CameraMode::ThirdPerson:
            return "2.5D";
        case CameraMode::FirstPerson:
            return "FIRST PERSON";
        case CameraMode::Spectator:
            return "OBSERVER";
    }

    return "CAMERA";
}

float interfaceScale(int width, int height) {
    return std::max(0.75f, std::min(
        static_cast<float>(width) / 1280.0f,
        static_cast<float>(height) / 720.0f
    ));
}

void centeredText(
    const std::string& value,
    float centerX,
    float y,
    float scale,
    const glm::vec3& color
) {
    UiRenderer::text(
        value,
        centerX - UiRenderer::textWidth(value, scale) * 0.5f,
        y,
        scale,
        color
    );
}
}

bool App::init(int width, int height, const char* title) {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW.\n";
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window.\n";
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetWindowSizeLimits(window, 960, 540, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, App::handleResize);
    glfwSetKeyCallback(window, App::handleKey);

    windowWidth = width;
    windowHeight = height;
    previousTime = glfwGetTime();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_FOG);
    glShadeModel(GL_FLAT);
    glClearColor(0.54f, 0.7f, 0.88f, 1.0f);

    const GLfloat fogColor[] = {0.54f, 0.7f, 0.88f, 1.0f};
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogi(GL_FOG_MODE, GL_EXP2);
    glFogf(GL_FOG_DENSITY, 0.018f);

    resize(width, height);
    const char* startCamera = std::getenv("ROBOT3D_CAMERA");
    if (startCamera && std::string(startCamera) == "first") {
        camera.setMode(CameraMode::FirstPerson);
    }
    if (startCamera && std::string(startCamera) == "spectator") {
        camera.setMode(CameraMode::Spectator);
    }

    gameState.setup(scene.enemyCount());
    updateWindowTitle();
    return true;
}

void App::run() {
    while (!glfwWindowShouldClose(window)) {
        const double currentTime = glfwGetTime();
        const float deltaTime = std::min(0.05f, static_cast<float>(currentTime - previousTime));
        previousTime = currentTime;

        update(deltaTime);
        render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void App::shutdown() {
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }

    glfwTerminate();
}

void App::resize(int width, int height) {
    windowWidth = width > 0 ? width : 1;
    windowHeight = height > 0 ? height : 1;

    glViewport(0, 0, windowWidth, windowHeight);
}

void App::update(float deltaTime) {
    if (screen != AppScreen::Playing) {
        return;
    }

    bool titleChanged = false;
    if (input.consumeCameraToggle()) {
        camera.toggleMode();
        titleChanged = true;
    }

    if (input.consumeAttack() && camera.mode() != CameraMode::Spectator) {
        robot.startAttack();
        scene.fireBullet(robot.position(), forwardFromYaw(robot.yaw()));
    }

    robot.update(deltaTime, input, camera.mode());
    if (camera.mode() != CameraMode::Spectator) {
        robot.setPosition(scene.resolvePlayerPosition(robot.position(), robot.collisionRadius()));
    }
    scene.update(deltaTime);
    titleChanged = gameState.update(scene.defeatedEnemyCount()) || titleChanged;
    if (titleChanged) {
        updateWindowTitle();
    }
}

void App::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderSky();
    setupProjection();

    glMatrixMode(GL_MODELVIEW);
    const glm::mat4 view = camera.viewMatrix(robot.position(), robot.yaw());
    glLoadMatrixf(glm::value_ptr(view));

    setupLights();

    scene.render(camera.mode() == CameraMode::ThirdPerson, robot.position());
    robot.render(camera.mode() != CameraMode::ThirdPerson);

    if (screen == AppScreen::Start) {
        renderStartScreen();
        return;
    }

    renderHud();
    if (screen == AppScreen::Paused) {
        renderPauseScreen();
    }
}

void App::setKey(int key, int action) {
    if (action == GLFW_RELEASE) {
        input.setKey(key, false);
        return;
    }

    if (action != GLFW_PRESS) {
        return;
    }

    if (screen == AppScreen::Start) {
        if (key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE) {
            startGame();
        } else if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        return;
    }

    if (screen == AppScreen::Paused) {
        if (key == GLFW_KEY_ENTER || key == GLFW_KEY_ESCAPE) {
            startGame();
        } else if (key == GLFW_KEY_R) {
            restartGame();
        } else if (key == GLFW_KEY_Q) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        return;
    }

    if (key == GLFW_KEY_ESCAPE) {
        input.clear();
        screen = AppScreen::Paused;
        updateWindowTitle();
        return;
    }

    input.setKey(key, true);
}

void App::startGame() {
    input.clear();
    screen = AppScreen::Playing;
    previousTime = glfwGetTime();
    updateWindowTitle();
}

void App::restartGame() {
    robot = Robot {};
    scene = Scene {};
    camera.setMode(CameraMode::ThirdPerson);
    gameState.setup(scene.enemyCount());
    startGame();
}

void App::setupProjection() const {
    const float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);

    glMatrixMode(GL_PROJECTION);

    if (camera.mode() == CameraMode::ThirdPerson) {
        const float viewHeight = 15.0f;
        const float viewWidth = viewHeight * aspect;
        glLoadIdentity();
        glOrtho(
            -viewWidth * 0.5f,
            viewWidth * 0.5f,
            -viewHeight * 0.5f,
            viewHeight * 0.5f,
            0.1f,
            200.0f
        );
    } else {
        const glm::mat4 projection = glm::perspective(glm::radians(68.0f), aspect, 0.1f, 200.0f);
        glLoadMatrixf(glm::value_ptr(projection));
    }

    glMatrixMode(GL_MODELVIEW);
}

void App::setupLights() const {
    const GLfloat lightPosition[] = {-7.0f, 12.0f, 6.0f, 1.0f};
    const GLfloat ambient[] = {0.38f, 0.42f, 0.44f, 1.0f};
    const GLfloat diffuse[] = {0.9f, 0.88f, 0.78f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
}

void App::renderSky() const {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, static_cast<double>(windowWidth), static_cast<double>(windowHeight), 0.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_FOG);
    glDisable(GL_CULL_FACE);

    glBegin(GL_QUADS);
    glColor3f(0.43f, 0.63f, 0.84f);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(static_cast<float>(windowWidth), 0.0f);
    glColor3f(0.76f, 0.9f, 0.98f);
    glVertex2f(static_cast<float>(windowWidth), static_cast<float>(windowHeight));
    glVertex2f(0.0f, static_cast<float>(windowHeight));
    glEnd();

    glEnable(GL_CULL_FACE);
    glEnable(GL_FOG);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void App::renderHud() const {
    const float scale = interfaceScale(windowWidth, windowHeight);
    const std::string progress =
        "TARGETS " + std::to_string(gameState.defeatedTargets()) + "/" +
        std::to_string(gameState.totalTargets());

    UiRenderer::begin(windowWidth, windowHeight);

    UiRenderer::rect(14.0f * scale, 14.0f * scale, 210.0f * scale, 38.0f * scale, Panel, 0.78f);
    UiRenderer::text(progress, 28.0f * scale, 26.0f * scale, 2.0f * scale, White);

    const std::string view = std::string("VIEW ") + cameraLabel(camera.mode());
    const float viewWidth = UiRenderer::textWidth(view, 1.8f * scale) + 28.0f * scale;
    const float viewX = static_cast<float>(windowWidth) - viewWidth - 14.0f * scale;
    UiRenderer::rect(viewX, 14.0f * scale, viewWidth, 38.0f * scale, Panel, 0.78f);
    UiRenderer::text(view, viewX + 14.0f * scale, 27.0f * scale, 1.8f * scale, White);

    const std::string controls = "WASD   SPACE   C   ESC";
    const float controlsWidth = UiRenderer::textWidth(controls, 1.4f * scale) + 28.0f * scale;
    const float controlsX = (static_cast<float>(windowWidth) - controlsWidth) * 0.5f;
    const float controlsY = static_cast<float>(windowHeight) - 36.0f * scale;
    UiRenderer::rect(controlsX, controlsY, controlsWidth, 24.0f * scale, Panel, 0.68f);
    UiRenderer::text(
        controls,
        controlsX + 14.0f * scale,
        controlsY + 7.0f * scale,
        1.4f * scale,
        White
    );

    if (gameState.cleared()) {
        const float bannerWidth = 220.0f * scale;
        const float bannerX = (static_cast<float>(windowWidth) - bannerWidth) * 0.5f;
        UiRenderer::rect(bannerX, 14.0f * scale, bannerWidth, 44.0f * scale, Blue, 0.9f);
        centeredText(
            "AREA CLEAR",
            static_cast<float>(windowWidth) * 0.5f,
            28.0f * scale,
            2.2f * scale,
            White
        );
    }

    UiRenderer::end();
}

void App::renderStartScreen() const {
    const float scale = interfaceScale(windowWidth, windowHeight);
    const float panelWidth = 400.0f * scale;
    const float panelHeight = 230.0f * scale;
    const float panelX = (static_cast<float>(windowWidth) - panelWidth) * 0.5f;
    const float panelY = (static_cast<float>(windowHeight) - panelHeight) * 0.5f;

    UiRenderer::begin(windowWidth, windowHeight);
    UiRenderer::rect(0.0f, 0.0f, static_cast<float>(windowWidth), static_cast<float>(windowHeight), Panel, 0.5f);
    UiRenderer::rect(panelX, panelY, panelWidth, panelHeight, Panel, 0.92f);

    centeredText("ROBOT 3D ROAMING", static_cast<float>(windowWidth) * 0.5f, panelY + 48.0f * scale, 3.4f * scale, White);
    centeredText("ENTER  START", static_cast<float>(windowWidth) * 0.5f, panelY + 126.0f * scale, 2.2f * scale, Blue);
    centeredText("ESC  EXIT", static_cast<float>(windowWidth) * 0.5f, panelY + 172.0f * scale, 1.7f * scale, Gray);

    UiRenderer::end();
}

void App::renderPauseScreen() const {
    const float scale = interfaceScale(windowWidth, windowHeight);
    const float panelWidth = 360.0f * scale;
    const float panelHeight = 250.0f * scale;
    const float panelX = (static_cast<float>(windowWidth) - panelWidth) * 0.5f;
    const float panelY = (static_cast<float>(windowHeight) - panelHeight) * 0.5f;

    UiRenderer::begin(windowWidth, windowHeight);
    UiRenderer::rect(0.0f, 0.0f, static_cast<float>(windowWidth), static_cast<float>(windowHeight), Panel, 0.62f);
    UiRenderer::rect(panelX, panelY, panelWidth, panelHeight, Panel, 0.94f);
    centeredText("PAUSED", static_cast<float>(windowWidth) * 0.5f, panelY + 42.0f * scale, 3.2f * scale, White);
    centeredText("ENTER  RESUME", static_cast<float>(windowWidth) * 0.5f, panelY + 112.0f * scale, 2.0f * scale, Blue);
    centeredText("R  RESTART", static_cast<float>(windowWidth) * 0.5f, panelY + 158.0f * scale, 1.8f * scale, White);
    centeredText("Q  EXIT", static_cast<float>(windowWidth) * 0.5f, panelY + 200.0f * scale, 1.7f * scale, Gray);

    UiRenderer::end();
}

void App::updateWindowTitle() {
    std::string nextTitle = "Robot 3D Roaming World";
    if (screen == AppScreen::Start) {
        nextTitle += " - Start";
    } else if (screen == AppScreen::Paused) {
        nextTitle += " - Paused";
    } else {
        nextTitle = gameState.title() + " - " + cameraLabel(camera.mode());
    }

    if (nextTitle == currentTitle || !window) {
        return;
    }

    currentTitle = nextTitle;
    glfwSetWindowTitle(window, currentTitle.c_str());
}

void App::handleResize(GLFWwindow* window, int width, int height) {
    auto* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    if (app) {
        app->resize(width, height);
    }
}

void App::handleKey(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)scancode;
    (void)mods;

    auto* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    if (app) {
        app->setKey(key, action);
    }
}
