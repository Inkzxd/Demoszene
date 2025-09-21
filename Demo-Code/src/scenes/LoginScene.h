#pragma once

#include <string>
#include <glm/glm.hpp>
#include <vector>
#include <functional>

#include "graphics/Font.h"
#include "audio/SoundManager.h"

/**
 * @brief Scene for simulating a login process with animated username and password entry.
 */
class LoginScene {
public:
    /**
     * @brief Stages of the login animation.
     */
    enum Stage { SHOW_USERNAME, WAIT_PASSWORD, SHOW_PASSWORD, VERIFYING, ACCESS_GRANTED, FINISHED };

    /**
     * @brief Constructor. Initializes the login scene.
     */
    LoginScene();

    /**
     * @brief Update the login scene animation and state.
     * @param deltaTime Time elapsed since last update.
     */
    void update(float deltaTime);

    /**
     * @brief Render the login scene, including username, password, and status messages.
     * @param font Font object for text rendering.
     * @param y Y position for text.
     * @param lineSpacing Line spacing for text.
     * @param time Current time.
     * @param color Text color.
     */
    void render(Font& font, float y, float lineSpacing, float time, const glm::vec3& color);

    /**
     * @brief Check if the login scene is finished.
     * @return True if finished, false otherwise.
     */
    bool isFinished() const { return stage == FINISHED; }

    /**
     * @brief Reset the login scene to its initial state.
     */
    void reset();

    /**
     * @brief Set a callback function to be called on each type event (e.g., play typing sound).
     * @param callback Function to call.
     */
    void setOnTypeCallback(std::function<void()> callback) { onTypeCallback = callback; }

private:
    Stage stage;                      // Current stage of the login process.
    std::string username, password;   // Username and password strings.
    std::string typedUsername, typedPassword; // Animated username and password.
    float timer;                      // Timer for animation.
    int charIndex;                    // Current character index for typing animation.
    float verifyingDots;              // Animation for verifying dots.
    float accessTimer;                // Timer for access granted stage.
    std::function<void()> onTypeCallback; // Callback for typing event.
};