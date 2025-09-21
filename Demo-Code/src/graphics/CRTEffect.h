#ifndef CRTEFFECT_H
#define CRTEFFECT_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "ShaderManager.h"


/**
 * @brief Class for creating a CRT effect using OpenGL.
 * This class handles the setup of a framebuffer, texture, and shader program to render a CRT effect.
 * It includes methods for initializing the effect, resizing the viewport, and rendering the effect.
 * Also includes constructors and destructors for resource management.
 * 
 * Implemented in CRTEffect.cpp.
 */
class CRTEffect {
public:
    CRTEffect();
    ~CRTEffect();
    
    bool initialize(int width, int height);
    void resize(int width, int height);
    void beginRender();
    void endRender();
    void render(float time);
    int getWidth();
    int getHeight();
    GLuint getScreenTexture() const { return screenTexture; }

private:
    GLuint fbo;
    GLuint screenTexture;
    GLuint quadVAO, quadVBO;
    GLuint shaderProgram;
    
    void setupQuad();
};

#endif // CRTEFFECT_H