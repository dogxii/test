#include "render/ImportedObjMap.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <sstream>
#include <unordered_map>

#include <GLFW/glfw3.h>

#include <glm/common.hpp>
#include <glm/geometric.hpp>
#include "render/ShapeRenderer.h"

namespace {
constexpr float GroundY = -1.42f;

int resolveIndex(int rawIndex, int count) {
    if (rawIndex > 0) {
        return rawIndex - 1;
    }

    if (rawIndex < 0) {
        return count + rawIndex;
    }

    return -1;
}

int parseIndex(const std::string& text, int count) {
    if (text.empty()) {
        return -1;
    }

    try {
        return resolveIndex(std::stoi(text), count);
    } catch (...) {
        return -1;
    }
}

int parseFaceToken(const std::string& token, int positionCount) {
    const std::size_t slash = token.find('/');
    const std::string positionText = slash == std::string::npos ? token : token.substr(0, slash);
    return parseIndex(positionText, positionCount);
}

glm::vec3 normalFromTriangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
    const glm::vec3 normal = glm::cross(b - a, c - a);
    const float length = glm::length(normal);

    if (length <= 0.0001f) {
        return {0.0f, 1.0f, 0.0f};
    }

    return normal / length;
}

glm::vec3 colorForMaterial(const std::string& material) {
    static const std::unordered_map<std::string, glm::vec3> colors {
        {"material_0", {0.9f, 0.76f, 0.5f}},
        {"material_1", {0.82f, 0.68f, 0.48f}},
        {"material_2", {0.52f, 0.34f, 0.16f}},
        {"material_5", {0.78f, 0.64f, 0.42f}},
        {"material_6", {0.84f, 0.7f, 0.48f}},
        {"material_7", {0.35f, 0.2f, 0.09f}},
        {"material_9", {0.58f, 0.4f, 0.2f}},
        {"material_10", {0.68f, 0.62f, 0.52f}},
        {"material_11", {0.48f, 0.43f, 0.34f}},
        {"material_12", {0.54f, 0.36f, 0.16f}},
        {"material_15", {0.7f, 0.64f, 0.54f}},
        {"material_17", {0.6f, 0.42f, 0.18f}},
        {"material_18", {0.08f, 0.07f, 0.06f}},
        {"material_20", {0.32f, 0.36f, 0.28f}},
        {"material_21", {0.4f, 0.44f, 0.34f}},
        {"material_25", {0.72f, 0.56f, 0.34f}},
        {"material_31", {0.34f, 0.38f, 0.3f}},
        {"material_33", {1.0f, 0.86f, 0.56f}}
    };

    const auto found = colors.find(material);
    if (found != colors.end()) {
        return found->second;
    }

    return {0.78f, 0.66f, 0.48f};
}

glm::vec3 transformSourceVertex(const glm::vec3& source, const glm::vec3& center, float scale) {
    return {
        (source.x - center.x) * scale,
        source.z * scale - 1.42f,
        -(source.y - center.y) * scale
    };
}

bool projectedHeight(
    const glm::vec3& point,
    const glm::vec3& a,
    const glm::vec3& b,
    const glm::vec3& c,
    float& height
) {
    const float denominator =
        (b.z - c.z) * (a.x - c.x) +
        (c.x - b.x) * (a.z - c.z);

    if (std::abs(denominator) <= 0.00001f) {
        return false;
    }

    const float u =
        ((b.z - c.z) * (point.x - c.x) +
         (c.x - b.x) * (point.z - c.z)) /
        denominator;
    const float v =
        ((c.z - a.z) * (point.x - c.x) +
         (a.x - c.x) * (point.z - c.z)) /
        denominator;
    const float w = 1.0f - u - v;

    if (u < -0.001f || v < -0.001f || w < -0.001f) {
        return false;
    }

    height = u * a.y + v * b.y + w * c.y;
    return true;
}

bool shouldHideTriangle(
    const glm::vec3& a,
    const glm::vec3& b,
    const glm::vec3& c,
    const glm::vec3& focusPosition
) {
    const float focusGround = GroundY + focusPosition.y;
    const float averageHeight = (a.y + b.y + c.y) / 3.0f;

    if (averageHeight <= focusGround + 0.55f) {
        return false;
    }

    const float minX = std::min({a.x, b.x, c.x});
    const float maxX = std::max({a.x, b.x, c.x});
    const float minZ = std::min({a.z, b.z, c.z});
    const float maxZ = std::max({a.z, b.z, c.z});
    const float corridorRadius = 1.1f;
    const float cameraSideZ = focusPosition.z + 6.2f;

    return maxX >= focusPosition.x - corridorRadius &&
           minX <= focusPosition.x + corridorRadius &&
           maxZ >= std::min(focusPosition.z, cameraSideZ) - corridorRadius &&
           minZ <= std::max(focusPosition.z, cameraSideZ) + corridorRadius;
}

}

bool ImportedObjMap::load(const std::string& path, float scale) {
    std::ifstream file(path);
    if (!file) {
        return false;
    }

    return loadStream(file, scale);
}

bool ImportedObjMap::loadFromMemory(std::string_view data, float scale) {
    std::istringstream stream {std::string(data)};
    return loadStream(stream, scale);
}

bool ImportedObjMap::loadStream(std::istream& file, float scale) {
    std::vector<glm::vec3> sourcePositions;
    glm::vec3 minPoint {1000000.0f};
    glm::vec3 maxPoint {-1000000.0f};
    std::string currentMaterial = "default";

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream stream(line);
        std::string type;
        stream >> type;

        if (type != "v") {
            continue;
        }

        glm::vec3 position {};
        stream >> position.x >> position.y >> position.z;
        sourcePositions.push_back(position);

        minPoint.x = std::min(minPoint.x, position.x);
        minPoint.y = std::min(minPoint.y, position.y);
        minPoint.z = std::min(minPoint.z, position.z);
        maxPoint.x = std::max(maxPoint.x, position.x);
        maxPoint.y = std::max(maxPoint.y, position.y);
        maxPoint.z = std::max(maxPoint.z, position.z);
    }

    if (sourcePositions.empty()) {
        return false;
    }

    const glm::vec3 center {
        (minPoint.x + maxPoint.x) * 0.5f,
        (minPoint.y + maxPoint.y) * 0.5f,
        0.0f
    };

    std::vector<glm::vec3> positions;
    positions.reserve(sourcePositions.size());
    for (const glm::vec3& sourcePosition : sourcePositions) {
        positions.push_back(transformSourceVertex(sourcePosition, center, scale));
    }

    glm::vec2 loadedMinBounds {positions.front().x, positions.front().z};
    glm::vec2 loadedMaxBounds {positions.front().x, positions.front().z};
    for (const glm::vec3& position : positions) {
        loadedMinBounds.x = std::min(loadedMinBounds.x, position.x);
        loadedMinBounds.y = std::min(loadedMinBounds.y, position.z);
        loadedMaxBounds.x = std::max(loadedMaxBounds.x, position.x);
        loadedMaxBounds.y = std::max(loadedMaxBounds.y, position.z);
    }

    file.clear();
    file.seekg(0);

    std::vector<Batch> loadedBatches;
    std::vector<WalkableTriangle> loadedWalkableTriangles;
    auto batchForMaterial = [&loadedBatches](const std::string& material) -> Batch& {
        for (Batch& batch : loadedBatches) {
            if (batch.material == material) {
                return batch;
            }
        }

        loadedBatches.push_back({material, colorForMaterial(material), {}});
        return loadedBatches.back();
    };

    while (std::getline(file, line)) {
        std::istringstream stream(line);
        std::string type;
        stream >> type;

        if (type == "usemtl") {
            stream >> currentMaterial;
            continue;
        }

        if (type != "f") {
            continue;
        }

        std::vector<int> face;
        std::string token;
        while (stream >> token) {
            face.push_back(parseFaceToken(token, static_cast<int>(positions.size())));
        }

        if (face.size() < 3) {
            continue;
        }

        Batch& batch = batchForMaterial(currentMaterial);
        for (std::size_t i = 1; i + 1 < face.size(); ++i) {
            const int triangle[] = {face[0], face[i], face[i + 1]};
            bool validTriangle = true;

            for (int index : triangle) {
                if (index < 0 || index >= static_cast<int>(positions.size())) {
                    validTriangle = false;
                    break;
                }
            }

            if (!validTriangle) {
                continue;
            }

            const glm::vec3& a = positions[static_cast<std::size_t>(triangle[0])];
            const glm::vec3& b = positions[static_cast<std::size_t>(triangle[1])];
            const glm::vec3& c = positions[static_cast<std::size_t>(triangle[2])];
            const glm::vec3 normal = normalFromTriangle(a, b, c);

            batch.vertices.push_back({a, normal});
            batch.vertices.push_back({b, normal});
            batch.vertices.push_back({c, normal});

            if (std::abs(normal.y) >= 0.55f) {
                loadedWalkableTriangles.push_back({a, b, c});
            }
        }
    }

    loadedBatches.erase(
        std::remove_if(
            loadedBatches.begin(),
            loadedBatches.end(),
            [](const Batch& batch) { return batch.vertices.empty(); }
        ),
        loadedBatches.end()
    );

    if (loadedBatches.empty()) {
        return false;
    }

    batches = std::move(loadedBatches);
    walkableTriangles = std::move(loadedWalkableTriangles);
    minBounds = loadedMinBounds;
    maxBounds = loadedMaxBounds;
    loaded = true;
    return true;
}

void ImportedObjMap::render(bool hideOverhead, const glm::vec3& focusPosition) const {
    if (!loaded) {
        return;
    }

    glDisable(GL_CULL_FACE);

    for (const Batch& batch : batches) {
        ShapeRenderer::setMaterial(batch.color, 8.0f);

        glBegin(GL_TRIANGLES);
        for (std::size_t i = 0; i + 2 < batch.vertices.size(); i += 3) {
            const Vertex& a = batch.vertices[i];
            const Vertex& b = batch.vertices[i + 1];
            const Vertex& c = batch.vertices[i + 2];

            if (hideOverhead &&
                shouldHideTriangle(a.position, b.position, c.position, focusPosition)) {
                continue;
            }

            const Vertex triangle[] {a, b, c};
            for (const Vertex& vertex : triangle) {
                glNormal3f(vertex.normal.x, vertex.normal.y, vertex.normal.z);
                glVertex3f(vertex.position.x, vertex.position.y, vertex.position.z);
            }
        }
        glEnd();
    }

    glEnable(GL_CULL_FACE);
}

bool ImportedObjMap::ready() const {
    return loaded;
}

glm::vec3 ImportedObjMap::resolveCollision(const glm::vec3& position, float radius) const {
    if (!loaded) {
        return position;
    }

    glm::vec2 resolved {position.x, position.z};

    resolved.x = std::clamp(resolved.x, minBounds.x + radius, maxBounds.x - radius);
    resolved.y = std::clamp(resolved.y, minBounds.y + radius, maxBounds.y - radius);

    glm::vec3 result {resolved.x, position.y, resolved.y};
    float surfaceHeight = 0.0f;
    if (surfaceHeightAt(result, 1.15f, surfaceHeight)) {
        result.y = surfaceHeight - GroundY;
    }

    return result;
}

glm::vec3 ImportedObjMap::placeOnSurface(const glm::vec3& position) const {
    if (!loaded) {
        return position;
    }

    glm::vec3 result = position;
    float surfaceHeight = 0.0f;
    if (surfaceHeightAt(result, std::numeric_limits<float>::max(), surfaceHeight)) {
        result.y = surfaceHeight - GroundY;
    }
    return result;
}

bool ImportedObjMap::surfaceHeightAt(
    const glm::vec3& position,
    float maxStep,
    float& height
) const {
    const float currentHeight = GroundY + position.y;
    float closestDistance = std::numeric_limits<float>::max();
    bool found = false;

    for (const WalkableTriangle& triangle : walkableTriangles) {
        float candidateHeight = 0.0f;
        if (!projectedHeight(position, triangle.a, triangle.b, triangle.c, candidateHeight)) {
            continue;
        }

        const float distance = std::abs(candidateHeight - currentHeight);
        if (distance <= maxStep && distance < closestDistance) {
            closestDistance = distance;
            height = candidateHeight;
            found = true;
        }
    }

    return found;
}
