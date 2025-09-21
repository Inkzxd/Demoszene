#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glad/glad.h>

#include "graphics/Font.h"
#include "graphics/ShaderManager.h"
#include "audio/SoundManager.h"

class Font;
class SoundManager;

/**
 * @brief Scene for locating and visualizing geographic positions with map and radar effects.
 */
class LocateScene {
   public:
    /**
     * @brief Constructor. Initializes the scene and loads map data.
     */
    LocateScene();

    /**
     * @brief Destructor. Releases OpenGL resources.
     */
    ~LocateScene();

    /**
     * @brief Update the scene state and animation.
     * @param deltaTime Time elapsed since last update.
     */
    void update(float deltaTime);

    /**
     * @brief Render the scene, including map, radar, and HUD text.
     * @param font Font object for text rendering.
     * @param y Y position for text.
     * @param lineSpacing Line spacing for text.
     * @param time Current time.
     * @param color Text color.
     * @param width Screen width.
     * @param height Screen height.
     */
    void render(Font& font, float y, float lineSpacing, float time, const glm::vec3& color, int width, int height);

    /**
     * @brief Check if the scene is finished.
     * @return True if finished, false otherwise.
     */
    bool isFinished() const { return finished; }

    /**
     * @brief Set the sound manager for playing sound effects.
     * @param mgr Pointer to SoundManager.
     */
    void setSoundManager(SoundManager* mgr) { soundManager = mgr; }

    /**
     * @brief Reset the scene to its initial state.
     */
    void reset();

   private:
    /**
     * @brief Draw a map using the given projection and parameters.
     */
    void drawMap(const std::vector<std::vector<glm::vec2>>& mapData, const glm::mat4& projection, float cx, float cy,
                 float scale, glm::vec4 color);

    /**
     * @brief Draw a circle at the given position.
     */
    void drawCircle(float cx, float cy, float r, glm::vec4 color, const glm::mat4& projection);

    /**
     * @brief Draw a point at the given position.
     */
    void drawPoint(float x, float y, float size, glm::vec4 color, const glm::mat4& projection);

    /**
     * @brief Draw a line between two points.
     */
    void drawLine(float x1, float y1, float x2, float y2, glm::vec4 color, const glm::mat4& projection);

    /**
     * @brief Draw a cross at the given position.
     */
    void drawCross(float x, float y, float len, glm::vec4 color, const glm::mat4& projection);

    /**
     * @brief Draw a pulsing effect at the center of each map polyline.
     */
    void drawMapPulseCenter(const std::vector<std::vector<glm::vec2>>& mapData, const glm::mat4& projection, float cx,
                            float cy, float scale, float pulseScale, glm::vec4 color);

    int currentStep;                // Current step in the locating sequence.
    float timer;                    // Timer for step transitions.
    bool finished;                  // Whether the scene is finished.
    float charTimer;                // Timer for character animation.
    int charIndex;                  // Current character index for text animation.
    std::vector<std::string> steps; // Step descriptions.

    float zoom, targetZoom;         // Current and target zoom levels.
    float centerX, centerY, targetCenterX, targetCenterY; // Current and target map center positions.

    float locateAnimTimer = 0.0f;   // Timer for locate animation.
    bool locatingStarted = false;   // Whether locating animation has started.

    // OpenGL resources
    unsigned int vao, vbo;          // Vertex array and buffer objects.
    unsigned int shaderProgram;     // Shader program for rendering.

    SoundManager* soundManager;     // Pointer to sound manager for playing sounds.
};