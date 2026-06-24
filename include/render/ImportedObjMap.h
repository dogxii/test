#pragma once

#include <istream>
#include <string>
#include <string_view>
#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

class ImportedObjMap {
public:
    bool load(const std::string& path, float scale);
    bool loadFromMemory(std::string_view data, float scale);
    void render(bool hideOverhead, const glm::vec3& focusPosition) const;
    bool ready() const;
    glm::vec3 resolveCollision(const glm::vec3& position, float radius) const;
    glm::vec3 placeOnSurface(const glm::vec3& position) const;

private:
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
    };

    struct Batch {
        std::string material;
        glm::vec3 color;
        std::vector<Vertex> vertices;
    };

    struct WalkableTriangle {
        glm::vec3 a;
        glm::vec3 b;
        glm::vec3 c;
    };

    std::vector<Batch> batches;
    std::vector<WalkableTriangle> walkableTriangles;
    glm::vec2 minBounds {0.0f};
    glm::vec2 maxBounds {0.0f};
    bool loaded = false;

    bool loadStream(std::istream& stream, float scale);
    bool surfaceHeightAt(const glm::vec3& position, float maxStep, float& height) const;
};
