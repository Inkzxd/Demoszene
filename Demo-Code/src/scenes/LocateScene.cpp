#include "LocateScene.h"

#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <sstream>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <fstream>
#include <sstream>

#include "json.hpp"
using json = nlohmann::json;

/**
 * @brief Map normalization parameters for longitude and latitude.
 */
struct MapNorm {
    float lon_min, lon_max, lat_min, lat_max;
};

// Germany and Saarland map normalization parameters
const MapNorm norm_de = {5.0f, 16.0f, 47.0f, 55.0f};
const MapNorm norm_saar = {6.35f, 7.45f, 49.11f, 49.65f};
const MapNorm norm_htw = {6.970, 6.978, 49.234, 49.242};

/**
 * @brief Load map data from a GeoJSON file and normalize coordinates.
 * @param filename Path to the GeoJSON file.
 * @param norm Normalization parameters.
 * @return Vector of polylines (each polyline is a vector of glm::vec2 points).
 */
std::vector<std::vector<glm::vec2>> loadMapFromGeoJSON(const std::string& filename, const MapNorm& norm) {
    std::vector<std::vector<glm::vec2>> map;
    std::ifstream infile(filename);
    json j;
    infile >> j;

    for (const auto& feature : j["features"]) {
        const auto& geom = feature["geometry"];
        std::string type = geom["type"];
        if (type == "Polygon") {
            for (const auto& ring : geom["coordinates"]) {
                std::vector<glm::vec2> poly;
                for (const auto& pt : ring) {
                    float x = (pt[0].get<float>() - norm.lon_min) / (norm.lon_max - norm.lon_min) * 2.0f - 1.0f;
                    float y = (pt[1].get<float>() - norm.lat_min) / (norm.lat_max - norm.lat_min) * 2.0f - 1.0f;
                    poly.emplace_back(x, y);
                }
                map.push_back(poly);
            }
        } else if (type == "MultiPolygon") {
            for (const auto& polygon : geom["coordinates"]) {
                for (const auto& ring : polygon) {
                    std::vector<glm::vec2> poly;
                    for (const auto& pt : ring) {
                        float x = (pt[0].get<float>() - norm.lon_min) / (norm.lon_max - norm.lon_min) * 2.0f - 1.0f;
                        float y = (pt[1].get<float>() - norm.lat_min) / (norm.lat_max - norm.lat_min) * 2.0f - 1.0f;
                        poly.emplace_back(x, y);
                    }
                    map.push_back(poly);
                }
            }
        } else if (type == "LineString") {
            std::vector<glm::vec2> poly;
            for (const auto& pt : geom["coordinates"]) {
                float x = (pt[0].get<float>() - norm.lon_min) / (norm.lon_max - norm.lon_min) * 2.0f - 1.0f;
                float y = (pt[1].get<float>() - norm.lat_min) / (norm.lat_max - norm.lat_min) * 2.0f - 1.0f;
                poly.emplace_back(x, y);
            }
            map.push_back(poly);
        }
    }
    return map;
}

// map data
std::vector<std::vector<glm::vec2>> worldMap = {{}};
std::vector<std::vector<glm::vec2>> germanyMap = {{}};
std::vector<std::vector<glm::vec2>> saarMap = {{}};
std::vector<std::vector<glm::vec2>> htwMap = {{}};

std::vector<std::vector<glm::vec2>> germanyMap_in_de_norm = {{}};
std::vector<std::vector<glm::vec2>> saarMap_in_de_norm = {{}};
std::vector<std::vector<glm::vec2>> saarbrücken_in_de_norm = {{}};

/**
 * @brief Constructor. Initializes LocateScene and loads map data.
 */
LocateScene::LocateScene()
    : currentStep(0),
      timer(0.0f),
      finished(false),
      charTimer(0.0f),
      charIndex(0),
      soundManager(nullptr),
      zoom(0.5f),
      targetZoom(1.0f),
      centerX(0.5f),
      centerY(0.5f),
      targetCenterX(0.5f),
      targetCenterY(0.5f) {
    steps = {"Locating: Earth", "Locating: Germany", "Locating: Saarbruecken", "Locating: HTW Saar"};
    shaderProgram = ShaderManager::loadShader("shaders/line.vert", "shaders/line.frag");
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    worldMap = loadMapFromGeoJSON("assets/maps/world.geo.json", norm_de);
    germanyMap = loadMapFromGeoJSON("assets/maps/germany.geo.json", norm_de);
    saarMap = loadMapFromGeoJSON("assets/maps/saarland.geo.json", norm_saar);
    htwMap = loadMapFromGeoJSON("assets/maps/htwsaar.geo.json", norm_htw);

    germanyMap_in_de_norm = loadMapFromGeoJSON("assets/maps/simpleGermany.geo.json", norm_de);
    saarMap_in_de_norm = loadMapFromGeoJSON("assets/maps/simpleSaarland.geo.json", norm_de);
    saarbrücken_in_de_norm = loadMapFromGeoJSON("assets/maps/saarbruecken.geo.json", norm_saar);
}

/**
 * @brief Destructor. Releases OpenGL resources.
 */
LocateScene::~LocateScene() {
    glDeleteProgram(shaderProgram);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}

/**
 * @brief Update the scene state, animation, and transitions.
 * @param deltaTime Time elapsed since last update.
 */
void LocateScene::update(float deltaTime) {
    if (finished) return;
    timer += deltaTime;
    charTimer += deltaTime;

    // map zoom and center position animation
    float lerpSpeed = 3.0f;  // adjust this value to control the speed of the zoom and center position animation
    zoom += (targetZoom - zoom) * lerpSpeed * deltaTime;
    centerX += (targetCenterX - centerX) * lerpSpeed * deltaTime;
    centerY += (targetCenterY - centerY) * lerpSpeed * deltaTime;
    if (charIndex < (int)steps[currentStep].size() && charTimer > 0.1f) {
        charIndex++;
        charTimer = 0.0f;
    }

    float eps = 0.01f;
    bool mapAnimFinished =
        fabs(zoom - targetZoom) < eps && fabs(centerX - targetCenterX) < eps && fabs(centerY - targetCenterY) < eps;

    // add up timer only if map animation is finished
    if (mapAnimFinished) {
        if (!locatingStarted) {
            locateAnimTimer = 0.0f;
            locatingStarted = true;
        } else {
            locateAnimTimer += deltaTime;
        }
    } else {
        locatingStarted = false;
        locateAnimTimer = 0.0f;
    }

    // change map based on timer and character index
    if (timer > 8.0f && charIndex == (int)steps[currentStep].size() && mapAnimFinished) {
        if (currentStep == 0) {
            currentStep = 1;
            targetZoom = 5.0f;
            targetCenterX = 0.4f;
            targetCenterY = 0.6f;
            timer = 0.0f;
            charIndex = 0;
            if (soundManager) soundManager->playSound(2);
        } else if (currentStep == 1) {
            currentStep = 2;
            targetZoom = 10.0f;
            targetCenterX = 0.6f;
            targetCenterY = 0.4f;
            timer = 0.0f;
            charIndex = 0;
            if (soundManager) soundManager->playSound(2);
        } else if (currentStep == 2) {
            currentStep = 3;
            targetZoom = 20.0f;
            targetCenterX = 0.5f;
            targetCenterY = 0.6f;
            timer = 0.0f;
            charIndex = 0;
            if (soundManager) soundManager->playSound(2);
        } else if (currentStep == 3) {
            finished = true;
        }
    }
}

/**
 * @brief Draw a map using the given projection and parameters.
 * @param mapData Map polylines.
 * @param projection Projection matrix.
 * @param cx Center X.
 * @param cy Center Y.
 * @param scale Map scale.
 * @param color Line color.
 */
void LocateScene::drawMap(const std::vector<std::vector<glm::vec2>>& mapData, const glm::mat4& projection,
                                     float cx, float cy, float scale, glm::vec4 color) {
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
    glUniform4fv(glGetUniformLocation(shaderProgram, "uColor"), 1, &color[0]);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    for (const auto& polyline : mapData) {
        std::vector<float> verts;
        for (const auto& pt : polyline) {
            verts.push_back(cx + pt.x * scale);
            verts.push_back(cy + pt.y * scale);
        }
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glDrawArrays(GL_LINE_STRIP, 0, verts.size() / 2);
    }
}

/**
 * @brief Draw a circle at the given position.
 * @param cx Center X.
 * @param cy Center Y.
 * @param r Radius.
 * @param color Circle color.
 * @param projection Projection matrix.
 */
void LocateScene::drawCircle(float cx, float cy, float r, glm::vec4 color, const glm::mat4& projection) {
    std::vector<float> verts;
    int N = 128;
    for (int i = 0; i < N; ++i) {
        float angle = i / float(N) * 2 * M_PI;
        verts.push_back(cx + cos(angle) * r);
        verts.push_back(cy + sin(angle) * r);
    }
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
    glUniform4fv(glGetUniformLocation(shaderProgram, "uColor"), 1, &color[0]);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glDrawArrays(GL_LINE_LOOP, 0, N);
}

/**
 * @brief Draw a point at the given position.
 * @param x X position.
 * @param y Y position.
 * @param size Point size.
 * @param color Point color.
 * @param projection Projection matrix.
 */
void LocateScene::drawPoint(float x, float y, float size, glm::vec4 color, const glm::mat4& projection) {
    float verts[2] = {x, y};
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
    glUniform4fv(glGetUniformLocation(shaderProgram, "uColor"), 1, &color[0]);
    glPointSize(size);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glDrawArrays(GL_POINTS, 0, 1);
}

/**
 * @brief Draw a line between two points.
 * @param x1 Start X.
 * @param y1 Start Y.
 * @param x2 End X.
 * @param y2 End Y.
 * @param color Line color.
 * @param projection Projection matrix.
 */
void LocateScene::drawLine(float x1, float y1, float x2, float y2, glm::vec4 color,
                                      const glm::mat4& projection) {
    float verts[4] = {x1, y1, x2, y2};
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
    glUniform4fv(glGetUniformLocation(shaderProgram, "uColor"), 1, &color[0]);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glDrawArrays(GL_LINES, 0, 2);
}

/**
 * @brief Draw a cross at the given position.
 * @param x Center X.
 * @param y Center Y.
 * @param len Half length of cross arms.
 * @param color Cross color.
 * @param projection Projection matrix.
 */
void LocateScene::drawCross(float x, float y, float len, glm::vec4 color, const glm::mat4& projection) {
    drawLine(x - len, y, x + len, y, color, projection);
    drawLine(x, y - len, x, y + len, color, projection);
}

/**
 * @brief Draw a pulsing effect at the center of each map polyline.
 * @param mapData Map polylines.
 * @param projection Projection matrix.
 * @param cx Center X.
 * @param cy Center Y.
 * @param scale Map scale.
 * @param pulseScale Pulse scale factor.
 * @param color Pulse color.
 */
void LocateScene::drawMapPulseCenter(const std::vector<std::vector<glm::vec2>>& mapData,
                                                const glm::mat4& projection, float cx, float cy, float scale,
                                                float pulseScale, glm::vec4 color) {
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
    glUniform4fv(glGetUniformLocation(shaderProgram, "uColor"), 1, &color[0]);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    for (const auto& polyline : mapData) {
        if (polyline.empty()) continue;
        // calculate center of the polyline
        float sumX = 0, sumY = 0;
        for (const auto& pt : polyline) {
            sumX += pt.x;
            sumY += pt.y;
        }
        float centerX = sumX / polyline.size();
        float centerY = sumY / polyline.size();

        std::vector<float> verts;
        for (const auto& pt : polyline) {
            float px = centerX + (pt.x - centerX) * pulseScale;
            float py = centerY + (pt.y - centerY) * pulseScale;
            verts.push_back(cx + px * scale);
            verts.push_back(cy + py * scale);
        }
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glDrawArrays(GL_LINE_STRIP, 0, verts.size() / 2);
    }
}

/**
 * @brief Render the LocateScene, including map, radar, and HUD text.
 * @param font Font object for text rendering.
 * @param y Y position for text.
 * @param lineSpacing Line spacing for text.
 * @param time Current time.
 * @param color Text color.
 * @param width Screen width.
 * @param height Screen height.
 */
void LocateScene::render(Font& font, float y, float lineSpacing, float time, const glm::vec3& color,
                                    int width, int height) {
    // background
    glClearColor(0.02f, 0.13f, 0.04f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // camera projection
    float viewW = width / zoom;
    float viewH = height / zoom;
    float cx = centerX * width;
    float cy = centerY * height;
    glm::mat4 projection = glm::ortho(cx - viewW / 2, cx + viewW / 2, cy - viewH / 2, cy + viewH / 2);

    // layered map rendering
    const std::vector<std::vector<glm::vec2>>* currentMap = &worldMap;
    float mapScale = 0.5f;
    MapNorm norm = norm_de;
    bool showHighlight = false;
    glm::vec4 highlightColor = glm::vec4(0.9f, 0.2f, 0.2f, 1.0f);

    if (currentStep == 0) {
        currentMap = &worldMap;
        mapScale = height * 0.02f;
        norm = norm_de;
        showHighlight = true;
    } else if (currentStep == 1) {
        currentMap = &germanyMap;
        mapScale = height * 0.08f; 
        norm = norm_de;
        showHighlight = true;
    } else if (currentStep == 2) {
        currentMap = &saarMap;
        mapScale = height * 0.04f;
        norm = norm_saar;
        showHighlight = true;
    } else if (currentStep == 3) {
        currentMap = &htwMap;
        mapScale = height * 0.02f; 
        norm = norm_htw;
    }
    drawMap(*currentMap, projection, cx, cy, mapScale, glm::vec4(0.2f, 1.0f, 0.6f, 0.7f));

    // check if map zoom is finished
    float radarFadeIn = glm::clamp(locateAnimTimer / 1.0f, 0.0f, 1.0f);
    float eps = 0.01f;
    bool mapAnimFinished =
        fabs(zoom - targetZoom) < eps && fabs(centerX - targetCenterX) < eps && fabs(centerY - targetCenterY) < eps;

    // radar animation only if map zooming animation is finished
    if (mapAnimFinished) {
        float radarCenterX = width / 2.0f;
        float radarCenterY = height / 2.0f;
        glm::mat4 screenProjection = glm::ortho(0.0f, float(width), 0.0f, float(height));
        for (int i = 1; i <= 4; ++i) {
            drawCircle(radarCenterX, radarCenterY, i * 80.0f + fmod(time * 60, 80.0f),
                       glm::vec4(1.0f, 0.2f, 0.2f, 1.0f * radarFadeIn), screenProjection);
        }

        // rader scan line
        float angle = fmod(time * 0.7f, 1.0f) * 2 * M_PI;
        float scanLen = std::max(width, height) * 0.7f;
        float sx = cx + cos(angle) * scanLen;
        float sy = cy + sin(angle) * scanLen;
        drawLine(cx, cy, sx, sy, glm::vec4(0, 1, 0, 0.8f * radarFadeIn), projection);

        // play radar sound every circle turn
        static float lastRadarSoundTime = -100.0f;
        float radarSoundInterval = 1.43f;  // approximately 1.43 seconds for a full circle
        if (time - lastRadarSoundTime >= radarSoundInterval) {
            if (soundManager) soundManager->playSound(5);
            lastRadarSoundTime = time;
        }

        // radar scan segments
        int segs = 32;
        float sweep = M_PI / 12;
        for (int i = 0; i < segs; ++i) {
            float a0 = angle - sweep / 2 + sweep * i / segs;
            float a1 = angle - sweep / 2 + sweep * (i + 1) / segs;
            float x0 = cx + cos(a0) * scanLen;
            float y0 = cy + sin(a0) * scanLen;
            float x1 = cx + cos(a1) * scanLen;
            float y1 = cy + sin(a1) * scanLen;
            drawLine(cx, cy, x0, y0, glm::vec4(0, 1, 0, 0.8f * radarFadeIn), projection);
            drawLine(cx, cy, x1, y1, glm::vec4(0, 1, 0, 0.8f * radarFadeIn), projection);
        }

        // locating data
        float targetX = cx, targetY = cy;
        float lon = 0.0f, lat = 0.0f;
        if (currentStep == 0) {
            lon = 10.4515f;
            lat = 51.1657f;
            norm = norm_de;
        } else if (currentStep == 1) {
            lon = 7.0f;
            lat = 49.4f;
            norm = norm_de;
        } else if (currentStep == 2) {
            lon = 6.992f;
            lat = 49.240f;
            norm = norm_saar;
        } else if (currentStep == 3) {
            lon = 6.973688439719524f;
            lat = 49.23571585936071f;
            norm = norm_htw;
        }
        float x = (lon - norm.lon_min) / (norm.lon_max - norm.lon_min) * 2.0f - 1.0f;
        float y_ = (lat - norm.lat_min) / (norm.lat_max - norm.lat_min) * 2.0f - 1.0f;
        targetX = cx + x * mapScale;
        targetY = cy + y_ * mapScale;

        float startX = cx, startY = cy;  // radar center position

        const float radarDelay = 1.5f;  // locate animation delay 1.5 seconds after radar starts
        float t = 0.0f;
        if (locateAnimTimer > radarDelay) {
            t = glm::clamp((locateAnimTimer - radarDelay) * 0.5f, 0.0f, 1.0f);
        }

        if (t > 0.0f) {
            float px = startX + (targetX - startX) * t;
            float py = startY + (targetY - startY) * t;

            // targeting path line
            glLineWidth(2.0f);
            drawLine(startX, startY, px, py, glm::vec4(1.0f, 0.95f, 0.4f, 1.0f), projection);
            glLineWidth(1.0f);

            // targeting point
            float screenTargetX = (px - (cx - viewW / 2)) / viewW * width;
            float screenTargetY = (py - (cy - viewH / 2)) / viewH * height;

            drawCross(screenTargetX, screenTargetY, 32.0f, glm::vec4(1.0f, 0.2f, 0.2f, 1.0f), screenProjection);
            drawCircle(screenTargetX, screenTargetY, 24.0f + 8.0f * fabs(sin(time * 2)),
                       glm::vec4(1.0f, 0.2f, 0.2f, 1.0f), screenProjection);
            glLineWidth(8.0f);
            drawCircle(screenTargetX, screenTargetY, 4.0f + 1.0f * fabs(sin(time * 2)),
                       glm::vec4(1.0f, 0.95f, 0.4f, 1.0f), screenProjection);
            glLineWidth(1.0f);

            // highlight the target area
            if (t >= 1.0f && showHighlight) {
                float pulsePeriod = 0.7f;
                float pulseTime = fmod(time, pulsePeriod);
                float pulseScale = (pulseTime < 0.15f) ? 1.05f : 1.0f;

                glLineWidth(5.0f);
                if (currentStep == 0) {
                    drawMapPulseCenter(germanyMap_in_de_norm, projection, cx, cy, mapScale, pulseScale, highlightColor);
                } else if (currentStep == 1) {
                    drawMapPulseCenter(saarMap_in_de_norm, projection, cx, cy, mapScale, pulseScale, highlightColor);
                } else if (currentStep == 2) {
                    drawMapPulseCenter(saarbrücken_in_de_norm, projection, cx, cy, mapScale, pulseScale,
                                       highlightColor);
                }
                glLineWidth(1.0f);
            }

            // HUD
            int signal = int(t * 100);
            std::stringstream hud;
            hud << "COORD: " << std::fixed << std::setprecision(6) << lon << ", " << lat << "   SIGNAL: " << signal
                << "%";
            font.renderText(hud.str(), cx - 180, cy + viewH / 2 - 40, 0.7f, glm::vec3(1.0f, 0.2f, 0.2f));
            std::string msg = steps[currentStep].substr(0, charIndex);
            font.renderText(msg, cx - 180, cy + viewH / 2 - 80, 1.0f, glm::vec3(0, 1, 0));
        }
    }
}

/**
 * @brief Reset the scene to its initial state.
 */
void LocateScene::reset() {
    currentStep = 0;
    timer = 0.0f;
    finished = false;
    charTimer = 0.0f;
    charIndex = 0;
    zoom = 1.0f;
    targetZoom = 1.0f;
    centerX = 0.5f;
    centerY = 0.5f;
    targetCenterX = 0.5f;
    targetCenterY = 0.5f;
    locateAnimTimer = 0.0f;
    locatingStarted = false;
}