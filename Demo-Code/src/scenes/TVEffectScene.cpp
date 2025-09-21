#include <glad/glad.h>
#include <iostream>

#include "TVEffectScene.h"

/**
 * @brief Constructor. Initializes TV effect scene state and OpenGL handles.
 */
TVEffectScene::TVEffectScene() : 
    shaderProgram(0), 
    vao(0), 
    vbo(0), 
    screenWidth(0), 
    screenHeight(0),
    finished(false),
    soundManager(nullptr),
    crtEffect(nullptr) {}

/**
 * @brief Destructor. Cleans up OpenGL resources.
 */
TVEffectScene::~TVEffectScene() {
    cleanup();
}

/**
 * @brief Initialize the TV effect scene with given screen size.
 * @param width Screen width.
 * @param height Screen height.
 * @return True if initialization succeeds, false otherwise.
 */
bool TVEffectScene::initialize(int width, int height) {
    screenWidth = width;
    screenHeight = height;
    if (!loadShader()) return false;
    createQuad();
    return true;
}

/**
 * @brief Resize the TV effect scene.
 * @param width New screen width.
 * @param height New screen height.
 */
void TVEffectScene::resize(int width, int height) {
    screenWidth = width;
    screenHeight = height;
}

/**
 * @brief Create a fullscreen quad for rendering the TV effect.
 */
void TVEffectScene::createQuad() {
    // Updated to include texture coordinates
    float quadVertices[] = {
        // positions   // texCoords
        0.0f, 0.0f,    0.0f, 0.0f,
        1.0f, 0.0f,    1.0f, 0.0f,
        1.0f, 1.0f,    1.0f, 1.0f,
        0.0f, 1.0f,    0.0f, 1.0f
    };

    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    
    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    
    // Texture coordinate attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    
    glBindVertexArray(0);
}

/**
 * @brief Load the shader program for the TV effect.
 * @return True if shader loads successfully, false otherwise.
 */
bool TVEffectScene::loadShader() {
    shaderProgram = ShaderManager::loadShader("shaders/tv_effect.vert", "shaders/tv_effect.frag");
    if (!shaderProgram) {
        std::cerr << "Failed to load TVEffectScene shader." << std::endl;
        return false;
    }
    return true;
}

/**
 * @brief Render the TV effect with CRT texture and animation.
 * @param time Current time (for animation).
 * @param closeAnim Animation progress (0=normal, 1=closed).
 */
void TVEffectScene::render(float time, float closeAnim) {
    if (!crtEffect) return;

    glUseProgram(shaderProgram);
    
    // Bind the CRT texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, crtEffect->getScreenTexture());
    glUniform1i(glGetUniformLocation(shaderProgram, "screenTexture"), 0);
    
    // Set other uniforms
    glUniform2f(glGetUniformLocation(shaderProgram, "iResolution"), 
                float(screenWidth), float(screenHeight));
    glUniform1f(glGetUniformLocation(shaderProgram, "iTime"), time);
    glUniform1f(glGetUniformLocation(shaderProgram, "closeAnim"), closeAnim);

    // Sound management
    static bool snowSoundPlayed = false;
    static bool biboiSoundPlayed = false;

    if (closeAnim <= 0.01f) {
        if (!snowSoundPlayed && soundManager) {
            snowSoundPlayed = true;
        }
        biboiSoundPlayed = false;
    } else if (closeAnim >= 1.0f) {
        if (!biboiSoundPlayed && soundManager) {
            biboiSoundPlayed = true;
        }
        snowSoundPlayed = false;
    } else {
        snowSoundPlayed = false;
        biboiSoundPlayed = false;
    }

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
}

/**
 * @brief Release OpenGL resources used by the TV effect scene.
 */
void TVEffectScene::cleanup() {
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (shaderProgram) glDeleteProgram(shaderProgram);
}