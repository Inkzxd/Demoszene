#pragma once

#include <glm/glm.hpp>
#include "audio/SoundManager.h"
#include "graphics/ShaderManager.h"
#include "graphics/CRTEffect.h"

class SoundManager;
class CRTEffect;

/**
 * @brief Scene for rendering a TV/CRT effect with animation and sound.
 */
class TVEffectScene
{
public:
    /**
     * @brief Constructor. Initializes the TV effect scene.
     */
    TVEffectScene();

    /**
     * @brief Destructor. Cleans up OpenGL resources.
     */
    ~TVEffectScene();

    /**
     * @brief Initialize the scene with given screen size.
     * @param width Screen width.
     * @param height Screen height.
     * @return True if initialization succeeds, false otherwise.
     */
    bool initialize(int width, int height);

    /**
     * @brief Resize the scene.
     * @param width New screen width.
     * @param height New screen height.
     */
    void resize(int width, int height);

    /**
     * @brief Render the TV effect.
     * @param time Current time (for animation).
     * @param closeAnim Animation progress (0=normal, 1=closed).
     */
    void render(float time, float closeAnim); 

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
     * @brief Set the CRT effect for screen texture.
     * @param effect Pointer to CRTEffect.
     */
    void setCRTEffect(CRTEffect* effect) { crtEffect = effect; }

    /**
     * @brief Release OpenGL resources.
     */
    void cleanup();

private:
    unsigned int shaderProgram;   ///< Shader program ID.
    unsigned int vao, vbo;        ///< Vertex array and buffer objects.
    int screenWidth, screenHeight;///< Screen size.
    bool finished;                ///< Whether the scene is finished.
    SoundManager* soundManager;   ///< Pointer to sound manager.
    CRTEffect* crtEffect;         ///< Pointer to CRT effect.

    /**
     * @brief Create a fullscreen quad for rendering.
     */
    void createQuad();

    /**
     * @brief Load the shader program.
     * @return True if shader loads successfully, false otherwise.
     */
    bool loadShader();
};