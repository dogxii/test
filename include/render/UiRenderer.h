#pragma once

#include <string>

#include <glm/vec3.hpp>

class UiRenderer {
public:
    static void begin(int width, int height);
    static void end();
    static void rect(float x, float y, float width, float height, const glm::vec3& color, float alpha = 1.0f);
    static void text(
        const std::string& value,
        float x,
        float y,
        float scale,
        const glm::vec3& color
    );
    static float textWidth(const std::string& value, float scale);
};
