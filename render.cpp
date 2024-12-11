#include "render.h"

RenderEngine::RenderEngine(std::unique_ptr<HeightGenerator_Creator> generator_creator, std::function<float(float, float)> noise)
    : fjord{ std::make_unique<Fjord>(generator_creator->create(std::move(noise))) },
    modelMatrix(glm::mat4(1.0f)),
    translation(glm::vec3(0.0f)),
    rotationAngle(0.0f),
    cameraDistance(1.0f),
    scaleFactor(1.0f),
    lightPos(glm::vec3(0.0f, 100.0f, 100.0f)) {}

void RenderEngine::update(
    bool _regen, int octave, int seed,
    int maxElevation, int tileSize, bool isLake, float waterPercentage) {
    fjord->update(
        _regen,
        octave,
        seed,
        maxElevation,
        tileSize,
        isLake,
        waterPercentage
    );
}

void RenderEngine::render() {
    glm::mat4 mvp = setupProjection();
    std::vector<std::vector<float>> zBuffer(ofGetWidth(), std::vector<float>(ofGetHeight(), FLT_MAX));

    const float screenWidth = ofGetWidth();
    const float screenHeight = ofGetHeight();

    /*
        Handle each square unit as two simplexes.
        For each simplex calculate:
            - Elevation
            - Normal
            - Coordinates on a screen using MVP matrix
    */
    int size = fjord->getSize();
    int tileSize = fjord->getTileSize();
    int maxElevation = fjord->getMaxElevation();
    std::vector<std::vector<float>> hmap = fjord->getHeightMap();


#pragma omp parallel for
    for (int j = 0; j < size - 1; ++j) {
        for (int i = 0; i < size - 1; ++i) {
            glm::vec3 vertices[6];

            vertices[0] = glm::vec3(i * tileSize, j * tileSize, hmap[j][i]);
            vertices[1] = glm::vec3((i + 1) * tileSize, j * tileSize, hmap[j][i + 1]);
            vertices[2] = glm::vec3(i * tileSize, (j + 1) * tileSize, hmap[j + 1][i]);
            vertices[3] = glm::vec3(i * tileSize, (j + 1) * tileSize, hmap[j + 1][i]);
            vertices[4] = glm::vec3((i + 1) * tileSize, j * tileSize, hmap[j][i + 1]);
            vertices[5] = glm::vec3((i + 1) * tileSize, (j + 1) * tileSize, hmap[j + 1][i + 1]);

            float simElev_1 = (hmap[j][i] + hmap[j][i + 1] + hmap[j + 1][i]) / 3;
            simElev_1 = ofMap(simElev_1, -maxElevation, maxElevation, 0, 1);

            float simElev_2 = (hmap[j + 1][i] + hmap[j][i + 1] + hmap[j + 1][i + 1]) / 3;
            simElev_2 = ofMap(simElev_2, -maxElevation, maxElevation, 0, 1);

            glm::vec3 normal_1 = glm::normalize(glm::cross(vertices[1] - vertices[0], vertices[2] - vertices[0]));
            glm::vec3 normal_2 = glm::normalize(glm::cross(vertices[4] - vertices[3], vertices[5] - vertices[3]));
            if (glm::length(normal_1) < 1e-6) {
                normal_1 = glm::vec3(0.0f, 0.0f, 1.0f);
            }
            if (glm::length(normal_2) < 1e-6) {
                normal_2 = glm::vec3(0.0f, 0.0f, 1.0f);
            }

            for (int k = 0; k < 2; ++k) {
                glm::vec4 screenCoords[3];
                glm::vec3* triangleVertices = &vertices[k * 3];

                for (int v = 0; v < 3; ++v) {
                    glm::vec4 worldCoord = glm::vec4(triangleVertices[v], 1.0f);
                    screenCoords[v] = mvp * worldCoord;
                    screenCoords[v] /= screenCoords[v].w;
                    screenCoords[v].x = (screenCoords[v].x + 1.0f) * 0.5f * screenWidth;
                    screenCoords[v].y = (1.0f - screenCoords[v].y) * 0.5f * screenHeight;
                }

                if (k == 0) {
                    rasterizeTriangle(screenCoords, zBuffer, i, j, simElev_1, normal_1);
                }
                else {
                    rasterizeTriangle(screenCoords, zBuffer, i, j, simElev_2, normal_2);
                }
            }
        }
    }
}

void RenderEngine::rasterizeTriangle(const glm::vec4 vertices[3], std::vector<std::vector<float>>& zBuffer, int i, int j, float elev, glm::vec3 norm) {
    int tileSize = fjord->getTileSize();
    float minX = std::max(0.0f, std::min({ vertices[0].x, vertices[1].x, vertices[2].x }));
    float maxX = std::min(static_cast<float>(ofGetWidth() - 1), std::max({ vertices[0].x, vertices[1].x, vertices[2].x }));
    float minY = std::max(0.0f, std::min({ vertices[0].y, vertices[1].y, vertices[2].y }));
    float maxY = std::min(static_cast<float>(ofGetHeight() - 1), std::max({ vertices[0].y, vertices[1].y, vertices[2].y }));

    glm::vec3 lightDir = glm::normalize(lightPos - glm::vec3(i * tileSize, j * tileSize, elev));

    glm::vec2 v0 = glm::vec2(vertices[1]) - glm::vec2(vertices[0]);
    glm::vec2 v1 = glm::vec2(vertices[2]) - glm::vec2(vertices[0]);
    float invDet = 1.0f / (v0.x * v1.y - v0.y * v1.x);

#pragma omp parallel for
    for (int y = static_cast<int>(minY); y <= static_cast<int>(maxY); ++y) {
        for (int x = static_cast<int>(minX); x <= static_cast<int>(maxX); ++x) {
            glm::vec2 p = glm::vec2(x, y) - glm::vec2(vertices[0]);
            float bary1 = (p.x * v1.y - p.y * v1.x) * invDet;
            float bary2 = (v0.x * p.y - v0.y * p.x) * invDet;
            float bary0 = 1.0f - bary1 - bary2;

            if (bary0 >= 0 && bary1 >= 0 && bary2 >= 0) {
                float depth = bary0 * vertices[0].z + bary1 * vertices[1].z + bary2 * vertices[2].z;

                if (depth < zBuffer[x][y]) {
                    zBuffer[x][y] = depth;

                    float dotProduct = glm::dot(norm, lightDir);
                    float intensity = glm::clamp(dotProduct, 0.3f, 1.0f);

#pragma omp critical
                    {
                        ofSetColor(calculateColor(elev, intensity));
                        ofDrawRectangle(x, y, 1, 1);
                    }
                }
            }
        }
    }
}

ofColor RenderEngine::calculateColor(float height, float lightIntensity) {
    static const std::map<int, std::vector<std::tuple<float, float, ofColor, ofColor>>> colorRanges = {
        {1, {
            {0.0f, 0.51f, ofColor(0, 0, 255), ofColor(0, 0, 255)},  // Water
            {0.51f, 0.53f, ofColor(220, 212, 156), ofColor(34, 139, 34)}, // Sand to Grass
            {0.53f, 0.6f, ofColor(34, 139, 34), ofColor(0, 102, 51)},     // Grass to Forest
            {0.6f, 0.85f, ofColor(0, 102, 51), ofColor(210, 180, 140)},   // Forest to Earth
            {0.85f, 0.9f, ofColor(210, 180, 140), ofColor(224, 224, 224)}, // Earth to Grey
            {0.9f, 1.0f, ofColor(224, 224, 224), ofColor(224, 224, 224)}  // Grey
        }},
        {-1, {
            {0.0f, 0.51f, ofColor(181, 211, 255), ofColor(181, 211, 255)}, // Ice
            {0.51f, 0.6f, ofColor(0, 102, 51), ofColor(32, 32,32)},       // Grey4 to Grey3
            {0.6f, 0.75f, ofColor(32, 32, 32), ofColor(224,224, 224)},    // Grey3 to Snow
            {0.75f, 0.9f, ofColor(224, 224, 224), ofColor(255, 255, 255)}, // Snow
            {0.9f, 1.0f, ofColor(255, 255, 255), ofColor(255, 255, 255)}   // Snow
        }}
    };

    ofColor color = ofColor(192, 192, 192);

    if (height >= 0 && colorRanges.count(mapType)) {
        const auto& ranges = colorRanges.at(mapType);
        for (const auto& [low, high, lowColor, highColor] : ranges) {
            if (height >= low && height < high) {
                color = interpolateColor(height, low, high, lowColor, highColor);
                break;
            }
        }
        color.r = static_cast<unsigned char>(color.r * lightIntensity);
        color.g = static_cast<unsigned char>(color.g * lightIntensity);
        color.b = static_cast<unsigned char>(color.b * lightIntensity);
    }

    return color;
}

ofColor RenderEngine::interpolateColor(float elev, float l, float h, ofColor lc, ofColor hc) {
    ofColor c;
    c.r = ofMap(elev, l, h, lc.r, hc.r, true);
    c.g = ofMap(elev, l, h, lc.g, hc.g, true);
    c.b = ofMap(elev, l, h, lc.b, hc.b, true);

    return c;
}



glm::mat4 RenderEngine::setupProjection() {
    /*
    Calculate Model-View-Projection matrix.
    It's used to project landscape to a screen with perspective and transform.
    */
    int size = fjord->getSize();
    int tileSize = fjord->getTileSize();
    const struct {
        float ratio;
        float fov;
        float np;
        float fp;
    } projConf = {
        static_cast<float>(ofGetWidth()) / ofGetHeight(),
        50.0f,
        1.0f,
        1000.0f
    };
    glm::mat4 projection = glm::perspective(glm::radians(projConf.fov), projConf.ratio, projConf.np, projConf.fp);

    glm::vec3 cameraPos(
        (size * tileSize) * 1.5f,
        (size * tileSize) * 0.75f,
        (size * tileSize) * 0.75f
    );
    glm::vec3 cameraTarget((size * tileSize) / 2.0f, (size * tileSize) / 2.0f, 0.0f);
    glm::vec3 upVector(0.0f, 0.0f, 1.0f);
    glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, upVector);

    lightPos = cameraPos;

    return projection * view * modelMatrix;
}

void RenderEngine::changeMapType() {
    mapType = -mapType;
}

void RenderEngine::rotate(bool clockwise) {
    int size = fjord->getSize();
    int tileSize = fjord->getTileSize();
    float deltaAngle = clockwise ? 5.0f : -5.0f;
    rotationAngle += deltaAngle;

    float cx = (size * tileSize) / 2.0f;
    float cy = (size * tileSize) / 2.0f;

    glm::vec3 center(cx, cy, 0.0f);

    glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), -center);
    glm::mat4 rotateMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(deltaAngle), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), center);

    modelMatrix = translateBack * rotateMatrix * translateToOrigin * modelMatrix;
}

void RenderEngine::zoom(bool zoomIn) {
    int size = fjord->getSize();
    int tileSize = fjord->getTileSize();
    float scaleDelta = zoomIn ? 0.1f : -0.1f;
    scaleFactor += scaleDelta;

    scaleFactor = glm::clamp(scaleFactor, 0.1f, 10.0f);

    float cx = (size * tileSize) / 2.0f;
    float cy = (size * tileSize) / 2.0f;

    glm::vec3 center(cx, cy, 0.0f);

    glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), -center);
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f + scaleDelta, 1.0f + scaleDelta, 1.0f));
    glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), center);

    modelMatrix = translateBack * scaleMatrix * translateToOrigin * modelMatrix;
}