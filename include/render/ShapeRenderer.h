#pragma once

#include <glm/vec3.hpp>

class ShapeRenderer {
public:
    static void setMaterial(const glm::vec3& color, float shininess = 16.0f);
    static void cube(float size = 1.0f);
    static void sphere(float radius, int slices = 24, int stacks = 16);
    static void torus(float majorRadius, float minorRadius, int majorSegments = 48, int minorSegments = 12);
    static void cone(float radius, float height, int segments = 8);
};
