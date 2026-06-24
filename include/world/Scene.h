#pragma once

#include <vector>

#include <glm/vec3.hpp>

struct EnemyDummy {
    glm::vec3 position;
    float yaw = 0.0f;
    bool defeated = false;
    float fallProgress = 0.0f;
};

struct Bullet {
    glm::vec3 position;
    glm::vec3 velocity;
    float lifetime = 0.0f;
    bool active = true;
};

class Scene {
public:
    Scene();

    void update(float deltaTime);
    void render(bool hideOverhead, const glm::vec3& focusPosition) const;
    void fireBullet(const glm::vec3& position, const glm::vec3& direction);
    int enemyCount() const;
    int defeatedEnemyCount() const;
    glm::vec3 resolvePlayerPosition(const glm::vec3& position, float radius) const;

private:
    std::vector<EnemyDummy> enemies;
    std::vector<Bullet> bullets;
    float orbitAngle = 0.0f;
    float ringAngle = 0.0f;

    void drawFloor() const;
    void drawPath() const;
    void drawWalls() const;
    void drawOrbitCore() const;
    void drawBullets() const;
    void drawProps() const;
    void drawTree(float x, float z, float scale) const;
    void drawGrassClump(float x, float z, float scale) const;
    void drawRock(float x, float z, float yaw, float scale) const;
    void drawEnemyDummy(const EnemyDummy& enemy) const;
    void drawFlag(float x, float z, float yaw, float scale) const;
    void drawHouse(float x, float z, float yaw, const glm::vec3& roofColor, float scale) const;
    void drawFence(float x, float z, float yaw, int posts) const;
};
