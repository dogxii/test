#include "render/ShapeRenderer.h"

#include <cmath>

#include <GLFW/glfw3.h>

#include <glm/geometric.hpp>
#include <glm/vec3.hpp>

namespace {
constexpr float PI = 3.1415926535f;

void normalFromPoint(float x, float y, float z) {
    const float length = std::sqrt(x * x + y * y + z * z);

    if (length <= 0.0001f) {
        glNormal3f(0.0f, 1.0f, 0.0f);
        return;
    }

    glNormal3f(x / length, y / length, z / length);
}

void normalFromTriangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
    const glm::vec3 normal = glm::normalize(glm::cross(b - a, c - a));
    glNormal3f(normal.x, normal.y, normal.z);
}
}

void ShapeRenderer::setMaterial(const glm::vec3& color, float shininess) {
    const GLfloat ambient[] = {color.r * 0.62f, color.g * 0.62f, color.b * 0.62f, 1.0f};
    const GLfloat diffuse[] = {color.r, color.g, color.b, 1.0f};
    const GLfloat specular[] = {0.06f, 0.06f, 0.06f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess * 0.35f);
}

void ShapeRenderer::cube(float size) {
    const float h = size * 0.5f;

    glBegin(GL_QUADS);

    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-h, -h, h);
    glVertex3f(h, -h, h);
    glVertex3f(h, h, h);
    glVertex3f(-h, h, h);

    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(h, -h, -h);
    glVertex3f(-h, -h, -h);
    glVertex3f(-h, h, -h);
    glVertex3f(h, h, -h);

    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-h, -h, -h);
    glVertex3f(-h, -h, h);
    glVertex3f(-h, h, h);
    glVertex3f(-h, h, -h);

    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(h, -h, h);
    glVertex3f(h, -h, -h);
    glVertex3f(h, h, -h);
    glVertex3f(h, h, h);

    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-h, h, h);
    glVertex3f(h, h, h);
    glVertex3f(h, h, -h);
    glVertex3f(-h, h, -h);

    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(-h, -h, -h);
    glVertex3f(h, -h, -h);
    glVertex3f(h, -h, h);
    glVertex3f(-h, -h, h);

    glEnd();
}

void ShapeRenderer::sphere(float radius, int slices, int stacks) {
    for (int stack = 0; stack < stacks; ++stack) {
        const float phi0 = -PI * 0.5f + PI * static_cast<float>(stack) / stacks;
        const float phi1 = -PI * 0.5f + PI * static_cast<float>(stack + 1) / stacks;

        glBegin(GL_QUAD_STRIP);
        for (int slice = 0; slice <= slices; ++slice) {
            const float theta = 2.0f * PI * static_cast<float>(slice) / slices;

            const float x0 = std::cos(phi0) * std::cos(theta);
            const float y0 = std::sin(phi0);
            const float z0 = std::cos(phi0) * std::sin(theta);

            const float x1 = std::cos(phi1) * std::cos(theta);
            const float y1 = std::sin(phi1);
            const float z1 = std::cos(phi1) * std::sin(theta);

            normalFromPoint(x0, y0, z0);
            glVertex3f(x0 * radius, y0 * radius, z0 * radius);

            normalFromPoint(x1, y1, z1);
            glVertex3f(x1 * radius, y1 * radius, z1 * radius);
        }
        glEnd();
    }
}

void ShapeRenderer::cone(float radius, float height, int segments) {
    const float halfHeight = height * 0.5f;
    const glm::vec3 tip {0.0f, halfHeight, 0.0f};
    const glm::vec3 baseCenter {0.0f, -halfHeight, 0.0f};

    glBegin(GL_TRIANGLES);
    for (int i = 0; i < segments; ++i) {
        const float a0 = static_cast<float>(i) / static_cast<float>(segments) * PI * 2.0f;
        const float a1 = static_cast<float>(i + 1) / static_cast<float>(segments) * PI * 2.0f;

        const glm::vec3 base0 {std::cos(a0) * radius, -halfHeight, std::sin(a0) * radius};
        const glm::vec3 base1 {std::cos(a1) * radius, -halfHeight, std::sin(a1) * radius};

        normalFromTriangle(tip, base1, base0);
        glVertex3f(tip.x, tip.y, tip.z);
        glVertex3f(base1.x, base1.y, base1.z);
        glVertex3f(base0.x, base0.y, base0.z);

        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(baseCenter.x, baseCenter.y, baseCenter.z);
        glVertex3f(base0.x, base0.y, base0.z);
        glVertex3f(base1.x, base1.y, base1.z);
    }
    glEnd();
}

void ShapeRenderer::torus(float majorRadius, float minorRadius, int majorSegments, int minorSegments) {
    for (int major = 0; major < majorSegments; ++major) {
        const float u0 = 2.0f * PI * static_cast<float>(major) / majorSegments;
        const float u1 = 2.0f * PI * static_cast<float>(major + 1) / majorSegments;

        glBegin(GL_QUAD_STRIP);
        for (int minor = 0; minor <= minorSegments; ++minor) {
            const float v = 2.0f * PI * static_cast<float>(minor) / minorSegments;

            const float cosV = std::cos(v);
            const float sinV = std::sin(v);

            const float x0 = (majorRadius + minorRadius * cosV) * std::cos(u0);
            const float y0 = minorRadius * sinV;
            const float z0 = (majorRadius + minorRadius * cosV) * std::sin(u0);

            const float x1 = (majorRadius + minorRadius * cosV) * std::cos(u1);
            const float y1 = minorRadius * sinV;
            const float z1 = (majorRadius + minorRadius * cosV) * std::sin(u1);

            glNormal3f(std::cos(u0) * cosV, sinV, std::sin(u0) * cosV);
            glVertex3f(x0, y0, z0);

            glNormal3f(std::cos(u1) * cosV, sinV, std::sin(u1) * cosV);
            glVertex3f(x1, y1, z1);
        }
        glEnd();
    }
}
