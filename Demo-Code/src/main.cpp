#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "graphics/CRTEffect.h"
#include "graphics/Font.h"
#include "graphics/ShaderManager.h"

#include "core/Config.h"
#include "core/WindowManager.h"

#include "scenes/LoginScene.h"
#include "scenes/LocateScene.h"
#include "scenes/TVEffectScene.h"
#include "scenes/TerminalScene.h"

#include "audio/SoundManager.h"


/**
 * @brief Easing function for ease-in quadratic animation.
 * @param t Normalized time (0-1).
 * @return Eased value.
 */
float easeInQuad(float t) {
    return t * t;
}

/**
 * @brief Easing function for ease-out quadratic animation.
 * @param t Normalized time (0-1).
 * @return Eased value.
 */
float easeOutQuad(float t) {
    return t * (2 - t);
}

/**
 * @brief Easing function for ease-in-out quadratic animation.
 * @param t Normalized time (0-1).
 * @return Eased value.
 */
float easeInOutQuad(float t) {
    return t < 0.5 ? 2 * t * t : -1 + (4 - 2 * t) * t;
}

/**
 * @brief GLFW framebuffer resize callback. Updates the OpenGL viewport.
 * @param window Pointer to GLFW window.
 * @param width New width.
 * @param height New height.
 */
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    if (height > 0) glViewport(0, 0, width, height);
}

/**
 * @brief GLFW key callback. Closes window on ESC key press.
 * @param window Pointer to GLFW window.
 * @param key Key code.
 * @param scancode Scan code.
 * @param action Key action.
 * @param mods Modifier keys.
 */
void close_window_on_escape(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
}

/**
 * @brief Main entry point. Initializes window, loads resources, and runs the main loop.
 * @return Exit code.
 */
int main() {
    if (!Config::load("config.ini")) {
        std::cerr << "Using default configuration values" << std::endl;
    }

    WindowManager windowManager;
    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primary);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    // Hier die neuen Config-Werte verwenden
    if (!windowManager.initialize(Config::fullscreen ? mode->width : Config::width,
                                  Config::fullscreen ? mode->height : Config::height, "Retro Terminal",
                                  Config::fullscreen)) {
        return -1;
    }

    windowManager.setKeyCallback(close_window_on_escape);
    windowManager.setFramebufferSizeCallback(framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_MULTISAMPLE);

    // Load font
    Font font;
    if (!font.load("assets/fonts/VT323-Regular.ttf", 50)) {
        std::cerr << "Failed to load font.\n";
        return -1;
    }

    // Initialize sound manager
    SoundManager soundManager;
    std::vector<std::string> soundFiles = {"assets/sounds/keystroke-01.wav",     // 0
                                           "assets/sounds/keystroke-02.wav",     // 1
                                           "assets/sounds/keystroke-03.wav",     // 2
                                           "assets/sounds/keystroke-04.wav",     // 3
                                           "assets/sounds/keystroke-05.wav",     // 4
                                           "assets/sounds/sonar.wav",            // 5
                                           "assets/sounds/plasma.wav",           // 6
                                           "assets/sounds/plasma_reverse.wav"};  // 7

    soundManager.loadSounds(soundFiles);

    if (!soundManager.loadSounds(soundFiles)) {
        std::cerr << "Failed to load sounds.\n";
        return -1;
    }

    if (!soundManager.loadBackgroundSound("assets/sounds/background.wav")) {
        std::cerr << "Failed to load background sound.\n";
        return -1;
    }

    LoginScene loginScene;
    LocateScene locateScene;
    locateScene.setSoundManager(&soundManager);

    TVEffectScene tvEffectScene;
    tvEffectScene.setSoundManager(&soundManager);

    int width, height;
    windowManager.getFramebufferSize(width, height);
    tvEffectScene.initialize(width, height);

    TerminalScene terminalAnimation;
    terminalAnimation.setOnTypeCallback([&soundManager]() { soundManager.playRandomSound(); });

    loginScene.setOnTypeCallback([&soundManager]() { soundManager.playRandomSound(); });

    CRTEffect crtEffect;
    if (!crtEffect.initialize(width, height)) {
        std::cerr << "Failed to initialize CRT effect.\n";
        return -1;
    }
    tvEffectScene.setCRTEffect(&crtEffect);

    glm::vec3 textColor(0.0f, 1.0f, 0.0f);  // Consistent green color
    int lastFontSize = 0;
    float lastTime = glfwGetTime();

    // State machine
    enum AppState {
        STATE_LOGIN,
        STATE_TERMINAL,
        STATE_COLLAPSE,
        STATE_BLACKSCREEN,
        STATE_REBUILD,
        STATE_LOCATE,
        STATE_RESET_COLLAPSE,
        STATE_RESET_BLACKSCREEN,
        STATE_RESET_REBUILD
    };

    AppState appState = STATE_LOGIN;
    float tvCloseAnim = 0.0f;
    float collapseStartTime = 0.0f;
    float blackScreenStartTime = 0.0f;
    float rebuildStartTime = 0.0f;
    float locateSceneStartTime = 0.0f;
    bool backgroundSoundPlaying = false;
    bool firstLocateEnter = true;

    int latFontSize = 0;
    int lastWidth = 0, lastHeight = 0;

    // Main loop
    while (!windowManager.shouldClose()) {
        float now = glfwGetTime();
        float deltaTime = now - lastTime;
        lastTime = now;

        windowManager.getFramebufferSize(width, height);

        // Dynamic font size adjustment
        if (width != lastWidth || height != lastHeight) {
            int fontPixelSize = std::max(16, height / 32);               // font size based on height
            font.load("assets/fonts/VT323-Regular.ttf", fontPixelSize);  
            lastFontSize = fontPixelSize;
            lastWidth = width;
            lastHeight = height;
            font.setScreenWidth(width);
        }

        if (width != crtEffect.getWidth() || height != crtEffect.getHeight()) {
            crtEffect.resize(width, height);
            tvEffectScene.resize(width, height);
        }

        // Clear screen with green background (slightly darker than text for contrast)
        glClearColor(0.0f, 0.1f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set up font rendering projection
        if (appState != STATE_BLACKSCREEN && appState != STATE_RESET_BLACKSCREEN) {
            glm::mat4 projection = glm::ortho(0.0f, float(width), 0.0f, float(height));
            glUseProgram(font.getShaderProgram());
            glUniformMatrix4fv(glGetUniformLocation(font.getShaderProgram(), "projection"), 1, GL_FALSE,
                               &projection[0][0]);

            font.setScreenWidth(width);
        }

        float lineSpacing = float(lastFontSize) * 1.0f;
        float y;

        // Begin rendering to CRT framebuffer
        if (appState != STATE_BLACKSCREEN && appState != STATE_RESET_BLACKSCREEN) {
            crtEffect.beginRender();
            glClearColor(0.0f, 0.1f, 0.0f, 1.0f);  // Maintain green background
            glClear(GL_COLOR_BUFFER_BIT);
        }

        switch (appState) {
            case STATE_LOGIN: {
                y = height / 2.0f;
                loginScene.update(deltaTime);
                loginScene.render(font, y, lineSpacing, now, textColor);

                if (loginScene.isFinished()) {
                    appState = STATE_TERMINAL;
                }

                if (!backgroundSoundPlaying) {
                    soundManager.playBackgroundSound();
                    backgroundSoundPlaying = true;
                }
                break;
            }

            case STATE_TERMINAL: {
                y = height - lastFontSize * 2;
                terminalAnimation.update(now);
                terminalAnimation.render(font, y, lineSpacing, now, textColor);

                if (terminalAnimation.isFinished()) {
                    appState = STATE_COLLAPSE;
                    collapseStartTime = now;
                    soundManager.playSound(6);  // Plasma sound
                }
                break;
            }

            case STATE_COLLAPSE: {
                // Smooth collapse animation with easing (0.75 seconds)
                float t = (now - collapseStartTime) / 0.75f;
                tvCloseAnim = easeInQuad(std::min(t, 1.0f));

                if (t >= 1.0f) {
                    appState = STATE_BLACKSCREEN;
                    blackScreenStartTime = now;
                }
                break;
            }

            case STATE_BLACKSCREEN: {
                // Stay black for 1 second with fade effect
                float t = (now - blackScreenStartTime) / 1.0f;
                if (t >= 1.0f) {
                    appState = STATE_REBUILD;
                    rebuildStartTime = now;
                    soundManager.playSound(7);  // Plasma reverse sound
                }
                break;
            }

            case STATE_REBUILD: {
                // Smooth rebuild animation with easing (1.25 seconds)
                float t = (now - rebuildStartTime) / 1.25f;
                tvCloseAnim = 1.0f - easeOutQuad(std::min(t, 1.0f));

                if (t >= 1.0f) {
                    appState = STATE_LOCATE;
                }
                break;
            }

            case STATE_LOCATE: {
                y = height / 2.0f;
                locateScene.update(deltaTime);
                locateScene.render(font, y, lineSpacing, now, textColor, width, height);

                if (firstLocateEnter) {
                    locateSceneStartTime = now;
                    firstLocateEnter = false;
                }

                // Transition after 3 seconds and when scene is finished
                if (now - locateSceneStartTime >= 3.0f && locateScene.isFinished()) {
                    soundManager.playSound(6);  // Plasma sound
                    appState = STATE_RESET_COLLAPSE;
                    collapseStartTime = now;
                    firstLocateEnter = true;
                }
                break;
            }

            case STATE_RESET_COLLAPSE: {
                // Smooth collapse animation with easing (0.75 seconds)
                float t = (now - collapseStartTime) / 0.75f;
                tvCloseAnim = easeInQuad(std::min(t, 1.0f));

                if (t >= 1.0f) {
                    appState = STATE_RESET_BLACKSCREEN;
                    blackScreenStartTime = now;
                }
                break;
            }

            case STATE_RESET_BLACKSCREEN: {
                // Stay black for 1 second with fade effect
                float t = (now - blackScreenStartTime) / 1.0f;
                if (t >= 1.0f) {
                    appState = STATE_RESET_REBUILD;
                    rebuildStartTime = now;
                    soundManager.playSound(7);  // Plasma reverse sound
                }
                break;
            }

            case STATE_RESET_REBUILD: {
                // Smooth rebuild animation with easing (1.25 seconds)
                float t = (now - rebuildStartTime) / 1.25f;
                tvCloseAnim = 1.0f - easeOutQuad(std::min(t, 1.0f));

                if (t >= 1.0f) {
                    // Reset all scenes
                    loginScene.reset();
                    terminalAnimation.reset();
                    locateScene.reset();

                    // Return to login state

                    appState = STATE_LOGIN;
                    backgroundSoundPlaying = false;
                    soundManager.stopBackgroundSound();
                }
                break;
            }
        }

        // End rendering to CRT buffer
        if (appState != STATE_BLACKSCREEN && appState != STATE_RESET_BLACKSCREEN) {
            crtEffect.endRender();
        }

        // Render final output
        if (appState == STATE_BLACKSCREEN || appState == STATE_RESET_BLACKSCREEN) {
            // Smooth black screen transition
            float t = (appState == STATE_BLACKSCREEN) ? (now - blackScreenStartTime) / 1.0f
                                                      : (now - blackScreenStartTime) / 1.0f;

            float fade =
                (appState == STATE_BLACKSCREEN) ? easeInQuad(std::min(t, 1.0f)) : easeInQuad(std::min(t, 1.0f));

            glClearColor(0.0f, 0.1f * (1.0f - fade), 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
        } else if (appState == STATE_COLLAPSE || appState == STATE_REBUILD || appState == STATE_RESET_COLLAPSE ||
                   appState == STATE_RESET_REBUILD) {
            // Render CRT effect with TV overlay
            crtEffect.render(now);
            tvEffectScene.render(now, tvCloseAnim);
        } else {
            // Normal scene rendering with CRT effect
            crtEffect.render(now);
        }

        windowManager.swapBuffers();
        windowManager.pollEvents();
    }
    tvEffectScene.cleanup();
    return 0;
}