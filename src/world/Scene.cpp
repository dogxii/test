#include "world/Scene.h"

#include <algorithm>
#include <cstddef>
#include <cmath>
#include <filesystem>
#include <string_view>

#include <GLFW/glfw3.h>

#include <glm/geometric.hpp>

#ifdef ROBOT3D_EMBEDDED_DUST2
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include "render/ImportedObjMap.h"
#include "render/ShapeRenderer.h"

namespace {
constexpr float PI = 3.1415926535f;

void drawCube(float x, float y, float z, float sx, float sy, float sz) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(sx, sy, sz);
    ShapeRenderer::cube();
    glPopMatrix();
}

void drawCubeYaw(float x, float y, float z, float sx, float sy, float sz, float yaw) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(yaw, 0.0f, 1.0f, 0.0f);
    glScalef(sx, sy, sz);
    ShapeRenderer::cube();
    glPopMatrix();
}

void drawRamp(float x, float z, float sx, float sz, float height, float yaw) {
    const float hx = sx * 0.5f;
    const float hz = sz * 0.5f;
    const float baseY = -1.4f;
    const float topY = baseY + height;

    glPushMatrix();
    glTranslatef(x, 0.0f, z);
    glRotatef(yaw, 0.0f, 1.0f, 0.0f);

    glBegin(GL_QUADS);

    glNormal3f(0.0f, 0.82f, -0.42f);
    glVertex3f(-hx, baseY, hz);
    glVertex3f(hx, baseY, hz);
    glVertex3f(hx, topY, -hz);
    glVertex3f(-hx, topY, -hz);

    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(-hx, baseY, -hz);
    glVertex3f(hx, baseY, -hz);
    glVertex3f(hx, baseY, hz);
    glVertex3f(-hx, baseY, hz);

    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(-hx, baseY, -hz);
    glVertex3f(hx, baseY, -hz);
    glVertex3f(hx, topY, -hz);
    glVertex3f(-hx, topY, -hz);

    glEnd();

    glBegin(GL_TRIANGLES);

    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-hx, baseY, hz);
    glVertex3f(-hx, topY, -hz);
    glVertex3f(-hx, baseY, -hz);

    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(hx, baseY, -hz);
    glVertex3f(hx, topY, -hz);
    glVertex3f(hx, baseY, hz);

    glEnd();

    glPopMatrix();
}

void drawShadow(float x, float z, float sx, float sz, float alpha, float y = -1.435f) {
    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.05f, 0.11f, 0.07f, alpha);

    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(sx, 1.0f, sz);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, 0.0f, 0.0f);
    for (int i = 0; i <= 18; ++i) {
        const float angle = static_cast<float>(i) / 18.0f * PI * 2.0f;
        glVertex3f(std::cos(angle), 0.0f, std::sin(angle));
    }
    glEnd();
    glPopMatrix();

    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glEnable(GL_FOG);
    glEnable(GL_LIGHTING);
}

void drawWallBlock(float x, float z, float sx, float sz, float height = 1.55f) {
    ShapeRenderer::setMaterial({0.86f, 0.74f, 0.54f});
    drawCube(x, -1.52f + height * 0.5f, z, sx, height, sz);

    ShapeRenderer::setMaterial({1.0f, 0.86f, 0.6f});
    drawCube(x, -1.52f + height + 0.08f, z, sx + 0.12f, 0.16f, sz + 0.12f);
}

void drawWallBlockYaw(float x, float z, float sx, float sz, float yaw, float height = 1.55f) {
    ShapeRenderer::setMaterial({0.86f, 0.74f, 0.54f});
    drawCubeYaw(x, -1.52f + height * 0.5f, z, sx, height, sz, yaw);

    ShapeRenderer::setMaterial({1.0f, 0.86f, 0.6f});
    drawCubeYaw(x, -1.52f + height + 0.08f, z, sx + 0.12f, 0.16f, sz + 0.12f, yaw);
}

void drawDoubleDoors(float x, float z, float yaw, float scale) {
    drawShadow(x, z, 0.65f * scale, 0.22f * scale, 0.1f);

    glPushMatrix();
    glTranslatef(x, -1.44f, z);
    glRotatef(yaw, 0.0f, 1.0f, 0.0f);
    glScalef(scale, scale, scale);

    ShapeRenderer::setMaterial({0.42f, 0.25f, 0.11f});
    drawCube(-0.18f, 0.58f, 0.0f, 0.28f, 1.15f, 0.12f);
    drawCube(0.18f, 0.58f, 0.0f, 0.28f, 1.15f, 0.12f);

    ShapeRenderer::setMaterial({0.24f, 0.14f, 0.08f});
    drawCube(0.0f, 0.6f, -0.08f, 0.06f, 1.1f, 0.06f);
    drawCube(-0.18f, 0.98f, -0.09f, 0.22f, 0.08f, 0.06f);
    drawCube(0.18f, 0.98f, -0.09f, 0.22f, 0.08f, 0.06f);

    glPopMatrix();
}

void drawCrates(float x, float z, float yaw, float scale) {
    drawShadow(x, z, 0.72f * scale, 0.48f * scale, 0.14f);

    glPushMatrix();
    glTranslatef(x, -1.42f, z);
    glRotatef(yaw, 0.0f, 1.0f, 0.0f);
    glScalef(scale, scale, scale);

    ShapeRenderer::setMaterial({0.55f, 0.38f, 0.18f});
    drawCube(-0.28f, 0.28f, 0.0f, 0.5f, 0.55f, 0.5f);
    drawCube(0.28f, 0.28f, 0.0f, 0.5f, 0.55f, 0.5f);
    drawCube(0.0f, 0.84f, 0.0f, 0.5f, 0.55f, 0.5f);

    ShapeRenderer::setMaterial({0.33f, 0.22f, 0.1f});
    drawCube(-0.28f, 0.56f, -0.27f, 0.52f, 0.08f, 0.06f);
    drawCube(0.28f, 0.56f, -0.27f, 0.52f, 0.08f, 0.06f);
    drawCube(0.0f, 1.12f, -0.27f, 0.52f, 0.08f, 0.06f);

    glPopMatrix();
}

void drawSiteMark(float x, float z, char label, const glm::vec3& color) {
    ShapeRenderer::setMaterial(color);
    drawCube(x, -1.36f, z, 2.15f, 0.035f, 2.15f);

    ShapeRenderer::setMaterial({0.98f, 0.94f, 0.78f});

    if (label == 'A') {
        drawCube(x - 0.34f, -1.31f, z, 0.16f, 0.035f, 1.15f);
        drawCube(x + 0.34f, -1.31f, z, 0.16f, 0.035f, 1.15f);
        drawCube(x, -1.3f, z + 0.05f, 0.78f, 0.035f, 0.16f);
        drawCube(x, -1.3f, z - 0.48f, 0.52f, 0.035f, 0.16f);
    } else {
        drawCube(x - 0.34f, -1.31f, z, 0.16f, 0.035f, 1.15f);
        drawCube(x + 0.08f, -1.3f, z - 0.42f, 0.68f, 0.035f, 0.16f);
        drawCube(x + 0.18f, -1.3f, z, 0.48f, 0.035f, 0.16f);
        drawCube(x + 0.08f, -1.3f, z + 0.42f, 0.68f, 0.035f, 0.16f);
        drawCube(x + 0.42f, -1.31f, z - 0.21f, 0.16f, 0.035f, 0.42f);
        drawCube(x + 0.42f, -1.31f, z + 0.21f, 0.16f, 0.035f, 0.42f);
    }
}

void drawArch(float x, float z, float yaw, float scale) {
    glPushMatrix();
    glTranslatef(x, -1.42f, z);
    glRotatef(yaw, 0.0f, 1.0f, 0.0f);
    glScalef(scale, scale, scale);

    ShapeRenderer::setMaterial({0.78f, 0.68f, 0.5f});
    drawCube(-0.62f, 0.7f, 0.0f, 0.22f, 1.4f, 0.34f);
    drawCube(0.62f, 0.7f, 0.0f, 0.22f, 1.4f, 0.34f);
    drawCube(0.0f, 1.42f, 0.0f, 1.46f, 0.28f, 0.36f);

    ShapeRenderer::setMaterial({0.58f, 0.48f, 0.34f});
    drawCube(0.0f, 1.62f, 0.0f, 1.68f, 0.12f, 0.42f);

    glPopMatrix();
}

bool loadEmbeddedDust2(ImportedObjMap& map) {
#ifdef ROBOT3D_EMBEDDED_DUST2
    constexpr int Dust2ResourceId = 101;
    constexpr int RawDataResourceType = 10;
    const HMODULE module = GetModuleHandleW(nullptr);
    const HRSRC resource = FindResourceW(
        module,
        MAKEINTRESOURCEW(Dust2ResourceId),
        MAKEINTRESOURCEW(RawDataResourceType)
    );
    if (!resource) {
        return false;
    }

    const HGLOBAL loadedResource = LoadResource(module, resource);
    const DWORD resourceSize = SizeofResource(module, resource);
    const void* resourceData = LockResource(loadedResource);
    if (!resourceData || resourceSize == 0) {
        return false;
    }

    return map.loadFromMemory(
        std::string_view(
            static_cast<const char*>(resourceData),
            static_cast<std::size_t>(resourceSize)
        ),
        0.008f
    );
#else
    static_cast<void>(map);
    return false;
#endif
}

ImportedObjMap& importedDust2Map() {
    static ImportedObjMap map;
    static bool attempted = false;

    if (attempted) {
        return map;
    }

    attempted = true;

    const std::filesystem::path paths[] {
        "de_dust2-cs-map/source/de_dust2/de_dust2.obj",
        "../de_dust2-cs-map/source/de_dust2/de_dust2.obj"
    };

    for (const std::filesystem::path& path : paths) {
        if (std::filesystem::exists(path) && map.load(path.string(), 0.008f)) {
            return map;
        }
    }

    loadEmbeddedDust2(map);
    return map;
}
}

Scene::Scene()
    : enemies {
        {{-12.0f, 0.0f, -12.0f}, 22.0f},
        {{0.0f, 0.0f, -12.0f}, 0.0f},
        {{12.0f, 0.0f, -8.0f}, -28.0f},
        {{-12.0f, 0.0f, 4.0f}, 18.0f},
        {{4.0f, 0.0f, 12.0f}, -36.0f},
        {{-8.0f, 0.0f, 12.0f}, 12.0f}
    } {
    ImportedObjMap& map = importedDust2Map();
    if (map.ready()) {
        for (EnemyDummy& enemy : enemies) {
            enemy.position = map.placeOnSurface(enemy.position);
        }
    }
}

void Scene::update(float deltaTime) {
    orbitAngle += deltaTime * 80.0f;
    ringAngle += deltaTime * 25.0f;

    for (EnemyDummy& enemy : enemies) {
        if (enemy.defeated) {
            enemy.fallProgress = std::min(1.0f, enemy.fallProgress + deltaTime * 4.0f);
        }
    }

    for (Bullet& bullet : bullets) {
        if (!bullet.active) {
            continue;
        }

        bullet.position += bullet.velocity * deltaTime;
        bullet.lifetime -= deltaTime;

        if (bullet.lifetime <= 0.0f) {
            bullet.active = false;
            continue;
        }

        const glm::vec3 flatBullet {bullet.position.x, 0.0f, bullet.position.z};
        for (EnemyDummy& enemy : enemies) {
            if (enemy.defeated) {
                continue;
            }

            const glm::vec3 flatEnemy {enemy.position.x, 0.0f, enemy.position.z};
            if (glm::distance(flatBullet, flatEnemy) <= 0.42f) {
                enemy.defeated = true;
                bullet.active = false;
                break;
            }
        }
    }

    bullets.erase(
        std::remove_if(
            bullets.begin(),
            bullets.end(),
            [](const Bullet& bullet) { return !bullet.active; }
        ),
        bullets.end()
    );
}

void Scene::render(bool hideOverhead, const glm::vec3& focusPosition) const {
    ImportedObjMap& map = importedDust2Map();
    if (map.ready()) {
        map.render(hideOverhead, focusPosition);
    } else {
        drawFloor();
        drawPath();
        drawWalls();
        drawProps();
    }

    for (const EnemyDummy& enemy : enemies) {
        drawEnemyDummy(enemy);
    }

    drawOrbitCore();
    drawBullets();
}

void Scene::fireBullet(const glm::vec3& position, const glm::vec3& direction) {
    const glm::vec3 flatDirection = glm::normalize(glm::vec3(direction.x, 0.0f, direction.z));
    const glm::vec3 bulletStart {
        position.x + flatDirection.x * 0.28f,
        -1.12f + position.y,
        position.z + flatDirection.z * 0.28f
    };
    bullets.push_back({bulletStart, flatDirection * 14.0f, 1.1f, true});
}

int Scene::enemyCount() const {
    return static_cast<int>(enemies.size());
}

int Scene::defeatedEnemyCount() const {
    return static_cast<int>(
        std::count_if(
            enemies.begin(),
            enemies.end(),
            [](const EnemyDummy& enemy) { return enemy.defeated; }
        )
    );
}

glm::vec3 Scene::resolvePlayerPosition(const glm::vec3& position, float radius) const {
    ImportedObjMap& map = importedDust2Map();

    if (map.ready()) {
        return map.resolveCollision(position, radius);
    }

    glm::vec3 resolved = position;
    resolved.x = std::clamp(resolved.x, -20.4f + radius, 20.4f - radius);
    resolved.z = std::clamp(resolved.z, -20.4f + radius, 20.4f - radius);
    return resolved;
}

void Scene::drawFloor() const {
    ShapeRenderer::setMaterial({0.82f, 0.7f, 0.5f});
    drawCube(0.0f, -1.55f, 0.0f, 110.0f, 0.2f, 110.0f);

    ShapeRenderer::setMaterial({0.66f, 0.56f, 0.39f});
    for (int x = -24; x <= 24; x += 4) {
        drawCube(static_cast<float>(x), -1.435f, -20.8f, 1.4f, 0.025f, 0.16f);
        drawCube(static_cast<float>(x) + 1.7f, -1.43f, 20.8f, 1.2f, 0.025f, 0.14f);
    }
}

void Scene::drawPath() const {
    ShapeRenderer::setMaterial({0.86f, 0.74f, 0.52f});
    drawCube(0.0f, -1.42f, 15.2f, 7.4f, 0.04f, 5.0f);
    drawCube(0.0f, -1.415f, 3.2f, 3.2f, 0.04f, 22.0f);
    drawCube(14.8f, -1.415f, 2.0f, 3.1f, 0.04f, 29.0f);
    drawCube(14.4f, -1.41f, -14.4f, 8.0f, 0.04f, 5.8f);
    drawCube(-14.6f, -1.41f, -14.0f, 7.2f, 0.04f, 5.8f);
    drawCube(-13.6f, -1.415f, 2.0f, 3.1f, 0.04f, 20.5f);
    drawCube(-7.2f, -1.412f, 7.4f, 10.8f, 0.04f, 2.4f);
    drawCube(7.4f, -1.413f, -8.8f, 11.0f, 0.04f, 2.3f);
    drawCube(7.4f, -1.412f, -5.4f, 3.2f, 0.04f, 6.2f);

    ShapeRenderer::setMaterial({0.62f, 0.52f, 0.38f});
    drawCube(0.0f, -1.385f, 3.2f, 0.22f, 0.02f, 21.4f);
    drawCube(14.8f, -1.385f, 2.0f, 0.24f, 0.02f, 28.4f);
    drawCube(-13.6f, -1.385f, 2.0f, 0.24f, 0.02f, 20.0f);

    ShapeRenderer::setMaterial({0.9f, 0.78f, 0.52f});
    drawRamp(10.6f, -10.8f, 5.2f, 5.8f, 0.72f, -34.0f);
    drawRamp(5.8f, -3.2f, 4.0f, 5.4f, 0.62f, -22.0f);
    drawRamp(-10.4f, -7.8f, 4.2f, 5.2f, 0.55f, 18.0f);
}

void Scene::drawWalls() const {
    for (int i = -22; i <= 22; i += 2) {
        drawWallBlock(static_cast<float>(i), -22.0f, 1.8f, 0.55f, 1.2f);
        drawWallBlock(static_cast<float>(i), 22.0f, 1.8f, 0.55f, 1.2f);
        drawWallBlock(-22.0f, static_cast<float>(i), 0.55f, 1.8f, 1.2f);
        drawWallBlock(22.0f, static_cast<float>(i), 0.55f, 1.8f, 1.2f);
    }

    drawWallBlock(-4.2f, -5.8f, 0.55f, 13.8f, 2.1f);
    drawWallBlock(4.6f, -5.0f, 0.55f, 12.2f, 2.0f);
    drawWallBlock(-18.2f, 1.0f, 0.65f, 13.2f, 1.85f);
    drawWallBlock(18.6f, 0.0f, 0.65f, 17.6f, 1.95f);
    drawWallBlock(7.8f, 6.4f, 13.6f, 0.55f, 1.7f);
    drawWallBlock(-7.2f, 9.4f, 11.5f, 0.55f, 1.65f);
    drawWallBlockYaw(-10.0f, -10.0f, 8.0f, 0.55f, -18.0f, 1.85f);
    drawWallBlockYaw(10.2f, -9.2f, 8.0f, 0.55f, 18.0f, 1.85f);

    drawWallBlock(13.0f, -16.9f, 4.4f, 0.55f, 1.45f);
    drawWallBlock(18.2f, -14.0f, 0.55f, 5.8f, 1.45f);
    drawWallBlock(-17.2f, -14.2f, 0.55f, 5.8f, 1.45f);
    drawWallBlock(-13.8f, -17.0f, 5.0f, 0.55f, 1.45f);

    drawWallBlock(-4.8f, 14.0f, 0.65f, 8.2f, 2.2f);
    drawWallBlock(4.8f, 14.0f, 0.65f, 8.2f, 2.2f);
    drawWallBlock(-2.9f, 8.7f, 2.4f, 0.55f, 2.05f);
    drawWallBlock(2.9f, 8.7f, 2.4f, 0.55f, 2.05f);
    drawWallBlock(0.0f, 6.2f, 5.8f, 0.55f, 1.2f);
}

void Scene::drawOrbitCore() const {
    glPushMatrix();
    glTranslatef(0.0f, 1.2f, -1.8f);
    glRotatef(ringAngle, 0.0f, 1.0f, 0.0f);
    ShapeRenderer::setMaterial({0.05f, 0.75f, 0.9f}, 18.0f);
    ShapeRenderer::torus(1.35f, 0.08f);
    glPopMatrix();

    const float radians = orbitAngle * PI / 180.0f;
    const float x = std::cos(radians) * 1.9f;
    const float z = -1.8f + std::sin(radians) * 1.9f;

    glPushMatrix();
    glTranslatef(x, 1.2f, z);
    ShapeRenderer::setMaterial({0.1f, 0.82f, 1.0f}, 22.0f);
    ShapeRenderer::sphere(0.28f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 1.2f, -1.8f);
    glRotatef(ringAngle * -1.4f, 0.0f, 1.0f, 0.0f);
    ShapeRenderer::setMaterial({0.12f, 0.25f, 0.32f});
    glScalef(0.5f, 0.5f, 0.5f);
    ShapeRenderer::cube();
    glPopMatrix();

    glDisable(GL_LIGHTING);
    glColor3f(0.05f, 0.75f, 0.9f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 64; ++i) {
        const float a = static_cast<float>(i) / 64.0f * PI * 2.0f;
        glVertex3f(std::cos(a) * 1.9f, 1.2f, -1.8f + std::sin(a) * 1.9f);
    }
    glEnd();
    glEnable(GL_LIGHTING);
}

void Scene::drawBullets() const {
    if (bullets.empty()) {
        return;
    }

    for (const Bullet& bullet : bullets) {
        glPushMatrix();
        glTranslatef(bullet.position.x, bullet.position.y, bullet.position.z);
        ShapeRenderer::setMaterial({1.0f, 0.78f, 0.12f}, 18.0f);
        ShapeRenderer::sphere(0.07f, 8, 6);
        glPopMatrix();
    }
}

void Scene::drawProps() const {
    drawSiteMark(14.4f, -14.2f, 'A', {0.72f, 0.2f, 0.16f});
    drawSiteMark(-14.8f, -14.0f, 'B', {0.18f, 0.5f, 0.72f});

    drawDoubleDoors(0.0f, 7.0f, 0.0f, 1.45f);
    drawDoubleDoors(9.4f, 8.4f, 90.0f, 1.2f);
    drawDoubleDoors(-13.6f, -2.8f, 0.0f, 1.2f);
    drawDoubleDoors(0.0f, -3.8f, 0.0f, 1.25f);

    drawArch(6.2f, -7.4f, 90.0f, 1.25f);
    drawArch(-14.2f, 3.2f, 0.0f, 1.15f);
    drawArch(14.7f, -7.8f, 0.0f, 1.1f);
    drawArch(0.0f, 11.1f, 0.0f, 1.25f);

    drawCrates(13.2f, -14.2f, 18.0f, 1.25f);
    drawCrates(16.5f, -12.8f, -12.0f, 1.05f);
    drawCrates(-14.8f, -14.0f, -18.0f, 1.2f);
    drawCrates(-12.8f, -11.5f, 25.0f, 0.95f);
    drawCrates(1.2f, -2.8f, 8.0f, 0.9f);
    drawCrates(15.8f, 3.4f, -10.0f, 0.82f);
    drawCrates(0.2f, 14.4f, 12.0f, 1.05f);

    drawHouse(-18.2f, -16.2f, 0.0f, {0.62f, 0.48f, 0.3f}, 1.15f);
    drawHouse(18.2f, -16.6f, 0.0f, {0.66f, 0.52f, 0.34f}, 1.2f);
    drawHouse(-18.4f, 7.2f, 8.0f, {0.58f, 0.46f, 0.32f}, 1.08f);
    drawHouse(18.4f, 10.2f, -12.0f, {0.62f, 0.48f, 0.34f}, 1.12f);
    drawHouse(4.8f, 17.0f, 0.0f, {0.58f, 0.45f, 0.3f}, 1.0f);

    drawFence(9.2f, -16.4f, 0.0f, 5);
    drawFence(-18.0f, -8.4f, 90.0f, 4);
    drawFence(18.2f, -1.0f, 90.0f, 5);

    drawFlag(13.0f, -17.6f, 0.0f, 0.9f);
    drawFlag(-17.1f, -16.8f, 0.0f, 0.9f);

    drawTree(-19.2f, 16.8f, 1.0f);
    drawTree(19.2f, 16.0f, 0.9f);
    drawTree(-20.0f, -18.4f, 0.8f);

    drawGrassClump(-12.8f, 11.8f, 1.0f);
    drawGrassClump(-4.8f, 15.0f, 0.75f);
    drawGrassClump(4.4f, 12.4f, 0.65f);
    drawGrassClump(10.8f, 8.6f, 0.9f);
    drawGrassClump(18.2f, 5.0f, 0.8f);
    drawGrassClump(-17.0f, -4.6f, 0.75f);
    drawGrassClump(8.6f, -15.2f, 0.85f);

    drawRock(-9.6f, 11.0f, 18.0f, 0.9f);
    drawRock(-2.4f, 6.2f, -28.0f, 0.74f);
    drawRock(9.0f, 0.8f, 45.0f, 1.0f);
    drawRock(17.2f, -7.0f, -8.0f, 0.8f);

    ShapeRenderer::setMaterial({0.28f, 0.22f, 0.16f});
    drawCube(-3.5f, -0.62f, 7.8f, 0.12f, 1.55f, 0.12f);
    drawCube(3.4f, -0.62f, 7.6f, 0.12f, 1.55f, 0.12f);
    drawCube(15.6f, -0.62f, 9.6f, 0.12f, 1.55f, 0.12f);

    ShapeRenderer::setMaterial({0.08f, 0.08f, 0.08f});
    drawCube(0.0f, 0.18f, 7.7f, 7.0f, 0.04f, 0.04f);
    drawCube(9.5f, 0.12f, 8.6f, 12.0f, 0.04f, 0.04f);
}

void Scene::drawTree(float x, float z, float scale) const {
    drawShadow(x, z, 0.75f * scale, 0.55f * scale, 0.14f);

    glPushMatrix();
    glTranslatef(x, -1.35f, z);
    glScalef(scale, scale, scale);

    ShapeRenderer::setMaterial({0.45f, 0.28f, 0.12f});
    drawCube(0.0f, 0.85f, 0.0f, 0.26f, 1.7f, 0.26f);

    ShapeRenderer::setMaterial({0.12f, 0.44f, 0.2f});
    for (int i = 0; i < 5; ++i) {
        const float yaw = static_cast<float>(i) * 72.0f;

        glPushMatrix();
        glTranslatef(0.0f, 1.82f, 0.0f);
        glRotatef(yaw, 0.0f, 1.0f, 0.0f);
        glRotatef(68.0f, 1.0f, 0.0f, 0.0f);
        ShapeRenderer::cone(0.22f, 1.3f, 4);
        glPopMatrix();
    }

    ShapeRenderer::setMaterial({0.68f, 0.44f, 0.18f});
    glPushMatrix();
    glTranslatef(0.0f, 1.72f, 0.0f);
    ShapeRenderer::sphere(0.18f, 10, 8);
    glPopMatrix();

    glPopMatrix();
}

void Scene::drawGrassClump(float x, float z, float scale) const {
    drawShadow(x, z, 0.35f * scale, 0.24f * scale, 0.08f);

    ShapeRenderer::setMaterial({0.63f, 0.52f, 0.28f});

    glPushMatrix();
    glTranslatef(x, -1.35f, z);
    glScalef(scale, scale, scale);

    glPushMatrix();
    glTranslatef(-0.24f, 0.32f, 0.02f);
    glRotatef(-16.0f, 0.0f, 0.0f, 1.0f);
    ShapeRenderer::cone(0.07f, 0.68f, 3);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.08f, 0.26f, 0.15f);
    glRotatef(11.0f, 1.0f, 0.0f, 0.0f);
    ShapeRenderer::cone(0.06f, 0.55f, 3);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.28f, 0.36f, -0.08f);
    glRotatef(18.0f, 0.0f, 0.0f, 1.0f);
    ShapeRenderer::cone(0.08f, 0.78f, 3);
    glPopMatrix();

    glPopMatrix();
}

void Scene::drawRock(float x, float z, float yaw, float scale) const {
    drawShadow(x, z, 0.58f * scale, 0.36f * scale, 0.12f);

    glPushMatrix();
    glTranslatef(x, -1.25f, z);
    glRotatef(yaw, 0.0f, 1.0f, 0.0f);
    glScalef(scale, scale, scale);

    ShapeRenderer::setMaterial({0.56f, 0.5f, 0.4f});
    drawCube(0.0f, 0.18f, 0.0f, 0.9f, 0.36f, 0.55f);

    ShapeRenderer::setMaterial({0.72f, 0.64f, 0.48f});
    drawCube(0.12f, 0.42f, -0.05f, 0.56f, 0.18f, 0.36f);

    glPopMatrix();
}

void Scene::drawEnemyDummy(const EnemyDummy& enemy) const {
    constexpr float scale = 0.176f;
    const float fallAngle = 84.0f * enemy.fallProgress;

    const float x = enemy.position.x;
    const float z = enemy.position.z;

    drawShadow(x, z, 0.42f * scale, 0.32f * scale, 0.16f, -1.435f + enemy.position.y);

    glPushMatrix();
    glTranslatef(x, -1.44f + enemy.position.y, z);
    glRotatef(enemy.yaw, 0.0f, 1.0f, 0.0f);
    glScalef(scale, scale, scale);
    if (enemy.defeated) {
        glTranslatef(0.0f, 0.0f, 0.1f);
        glRotatef(fallAngle, 1.0f, 0.0f, 0.0f);
        glTranslatef(0.0f, 0.0f, -0.1f);
    }

    const glm::vec3 bodyColor = enemy.defeated
        ? glm::vec3 {0.34f, 0.12f, 0.12f}
        : glm::vec3 {0.62f, 0.16f, 0.2f};
    ShapeRenderer::setMaterial(bodyColor);
    drawCube(0.0f, 0.95f, 0.0f, 0.62f, 0.9f, 0.36f);
    drawCube(0.0f, 1.65f, 0.0f, 0.58f, 0.58f, 0.5f);
    drawCube(-0.46f, 0.9f, 0.0f, 0.22f, 0.75f, 0.26f);
    drawCube(0.46f, 0.9f, 0.0f, 0.22f, 0.75f, 0.26f);
    drawCube(-0.18f, 0.25f, 0.0f, 0.22f, 0.5f, 0.26f);
    drawCube(0.18f, 0.25f, 0.0f, 0.22f, 0.5f, 0.26f);

    ShapeRenderer::setMaterial({0.96f, 0.98f, 0.9f}, 4.0f);
    drawCube(-0.12f, 1.7f, -0.27f, 0.1f, 0.09f, 0.04f);
    drawCube(0.12f, 1.7f, -0.27f, 0.1f, 0.09f, 0.04f);

    ShapeRenderer::setMaterial({0.05f, 0.04f, 0.04f}, 4.0f);
    drawCube(-0.1f, 1.69f, -0.295f, 0.04f, 0.04f, 0.025f);
    drawCube(0.1f, 1.69f, -0.295f, 0.04f, 0.04f, 0.025f);

    glPopMatrix();
}

void Scene::drawFlag(float x, float z, float yaw, float scale) const {
    drawShadow(x, z, 0.42f * scale, 0.28f * scale, 0.1f);

    glPushMatrix();
    glTranslatef(x, -1.42f, z);
    glRotatef(yaw, 0.0f, 1.0f, 0.0f);
    glScalef(scale, scale, scale);

    ShapeRenderer::setMaterial({0.2f, 0.22f, 0.22f});
    drawCube(0.0f, 0.78f, 0.0f, 0.12f, 1.55f, 0.12f);

    ShapeRenderer::setMaterial({0.76f, 0.2f, 0.14f});
    drawCube(0.38f, 1.34f, 0.0f, 0.7f, 0.36f, 0.08f);

    glPopMatrix();
}

void Scene::drawHouse(float x, float z, float yaw, const glm::vec3& roofColor, float scale) const {
    drawShadow(x, z, 1.35f * scale, 1.0f * scale, 0.16f);

    glPushMatrix();
    glTranslatef(x, -1.42f, z);
    glRotatef(yaw, 0.0f, 1.0f, 0.0f);
    glScalef(scale, scale, scale);

    ShapeRenderer::setMaterial({0.82f, 0.74f, 0.58f});
    drawCube(0.0f, 0.72f, 0.0f, 1.85f, 1.35f, 1.45f);

    ShapeRenderer::setMaterial(roofColor);
    drawCube(0.0f, 1.46f, 0.0f, 2.05f, 0.18f, 1.65f);
    drawCube(-0.55f, 1.66f, -0.28f, 0.25f, 0.25f, 0.25f);
    drawCube(0.46f, 1.66f, 0.3f, 0.22f, 0.22f, 0.22f);

    ShapeRenderer::setMaterial({0.14f, 0.1f, 0.08f});
    drawCube(0.0f, 0.34f, -0.7f, 0.42f, 0.68f, 0.08f);

    ShapeRenderer::setMaterial({0.22f, 0.28f, 0.32f});
    drawCube(-0.58f, 0.86f, -0.74f, 0.28f, 0.3f, 0.06f);
    drawCube(0.58f, 0.86f, -0.74f, 0.28f, 0.3f, 0.06f);

    ShapeRenderer::setMaterial({0.36f, 0.28f, 0.18f});
    drawCube(-0.58f, 0.86f, -0.78f, 0.34f, 0.04f, 0.04f);
    drawCube(0.58f, 0.86f, -0.78f, 0.34f, 0.04f, 0.04f);

    glPopMatrix();
}

void Scene::drawFence(float x, float z, float yaw, int posts) const {
    glPushMatrix();
    glTranslatef(x, -1.42f, z);
    glRotatef(yaw, 0.0f, 1.0f, 0.0f);

    ShapeRenderer::setMaterial({0.46f, 0.28f, 0.12f});

    const float spacing = 0.55f;
    const float start = -spacing * static_cast<float>(posts - 1) * 0.5f;

    for (int i = 0; i < posts; ++i) {
        drawCube(start + spacing * static_cast<float>(i), 0.45f, 0.0f, 0.12f, 0.9f, 0.12f);
    }

    drawCube(0.0f, 0.62f, 0.0f, spacing * static_cast<float>(posts), 0.1f, 0.1f);
    drawCube(0.0f, 0.28f, 0.0f, spacing * static_cast<float>(posts), 0.1f, 0.1f);

    glPopMatrix();
}
