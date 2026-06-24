#include "render/UiRenderer.h"

#include <array>
#include <cctype>
#include <unordered_map>

#include <GLFW/glfw3.h>

namespace {
using Glyph = std::array<unsigned char, 7>;

const Glyph& glyphFor(char character) {
    static const std::unordered_map<char, Glyph> glyphs {
        {'A', {14, 17, 17, 31, 17, 17, 17}},
        {'B', {30, 17, 17, 30, 17, 17, 30}},
        {'C', {14, 17, 16, 16, 16, 17, 14}},
        {'D', {30, 17, 17, 17, 17, 17, 30}},
        {'E', {31, 16, 16, 30, 16, 16, 31}},
        {'F', {31, 16, 16, 30, 16, 16, 16}},
        {'G', {14, 17, 16, 23, 17, 17, 15}},
        {'H', {17, 17, 17, 31, 17, 17, 17}},
        {'I', {31, 4, 4, 4, 4, 4, 31}},
        {'J', {7, 2, 2, 2, 18, 18, 12}},
        {'K', {17, 18, 20, 24, 20, 18, 17}},
        {'L', {16, 16, 16, 16, 16, 16, 31}},
        {'M', {17, 27, 21, 21, 17, 17, 17}},
        {'N', {17, 25, 21, 19, 17, 17, 17}},
        {'O', {14, 17, 17, 17, 17, 17, 14}},
        {'P', {30, 17, 17, 30, 16, 16, 16}},
        {'Q', {14, 17, 17, 17, 21, 18, 13}},
        {'R', {30, 17, 17, 30, 20, 18, 17}},
        {'S', {15, 16, 16, 14, 1, 1, 30}},
        {'T', {31, 4, 4, 4, 4, 4, 4}},
        {'U', {17, 17, 17, 17, 17, 17, 14}},
        {'V', {17, 17, 17, 17, 17, 10, 4}},
        {'W', {17, 17, 17, 21, 21, 21, 10}},
        {'X', {17, 17, 10, 4, 10, 17, 17}},
        {'Y', {17, 17, 10, 4, 4, 4, 4}},
        {'Z', {31, 1, 2, 4, 8, 16, 31}},
        {'0', {14, 17, 19, 21, 25, 17, 14}},
        {'1', {4, 12, 4, 4, 4, 4, 14}},
        {'2', {14, 17, 1, 2, 4, 8, 31}},
        {'3', {30, 1, 1, 14, 1, 1, 30}},
        {'4', {2, 6, 10, 18, 31, 2, 2}},
        {'5', {31, 16, 16, 30, 1, 1, 30}},
        {'6', {14, 16, 16, 30, 17, 17, 14}},
        {'7', {31, 1, 2, 4, 8, 8, 8}},
        {'8', {14, 17, 17, 14, 17, 17, 14}},
        {'9', {14, 17, 17, 15, 1, 1, 14}},
        {'-', {0, 0, 0, 31, 0, 0, 0}},
        {'.', {0, 0, 0, 0, 0, 12, 12}},
        {':', {0, 12, 12, 0, 12, 12, 0}},
        {'/', {1, 2, 2, 4, 8, 8, 16}},
        {'>', {16, 8, 4, 2, 4, 8, 16}},
        {'+', {0, 4, 4, 31, 4, 4, 0}},
        {' ', {0, 0, 0, 0, 0, 0, 0}}
    };
    static const Glyph fallback {31, 17, 1, 6, 4, 0, 4};

    const char normalized = static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
    const auto found = glyphs.find(normalized);
    return found == glyphs.end() ? fallback : found->second;
}
}

void UiRenderer::begin(int width, int height) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, static_cast<double>(width), static_cast<double>(height), 0.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_FOG);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void UiRenderer::end() {
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glEnable(GL_FOG);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void UiRenderer::rect(
    float x,
    float y,
    float width,
    float height,
    const glm::vec3& color,
    float alpha
) {
    glColor4f(color.r, color.g, color.b, alpha);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

void UiRenderer::text(
    const std::string& value,
    float x,
    float y,
    float scale,
    const glm::vec3& color
) {
    glColor3f(color.r, color.g, color.b);
    glBegin(GL_QUADS);

    float cursorX = x;
    for (char character : value) {
        const Glyph& glyph = glyphFor(character);
        for (std::size_t row = 0; row < glyph.size(); ++row) {
            for (int column = 0; column < 5; ++column) {
                if ((glyph[row] & (1 << (4 - column))) == 0) {
                    continue;
                }

                const float left = cursorX + static_cast<float>(column) * scale;
                const float top = y + static_cast<float>(row) * scale;
                glVertex2f(left, top);
                glVertex2f(left + scale, top);
                glVertex2f(left + scale, top + scale);
                glVertex2f(left, top + scale);
            }
        }
        cursorX += scale * 6.0f;
    }

    glEnd();
}

float UiRenderer::textWidth(const std::string& value, float scale) {
    if (value.empty()) {
        return 0.0f;
    }
    return static_cast<float>(value.size() * 6 - 1) * scale;
}
