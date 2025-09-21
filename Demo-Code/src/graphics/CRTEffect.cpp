#include "CRTEffect.h"
#include <iostream>

/**
 * @brief Constructor for CRTEffect class.
 * Initializes member variables to zero.
 * This sets up the OpenGL Framebuffer Object (FBO), screen texture, and shader program.
 */
CRTEffect::CRTEffect() : fbo(0), screenTexture(0), quadVAO(0), quadVBO(0), shaderProgram(0) {}

/**
 * @brief Destructor for CRTEffect class.
 * Cleans up OpenGL resources by deleting the framebuffer, texture, vertex array object, vertex buffer object and shader program.
 */
CRTEffect::~CRTEffect() {
    if (quadVAO) glDeleteVertexArrays(1, &quadVAO);
    if (quadVBO) glDeleteBuffers(1, &quadVBO);
    if (screenTexture) glDeleteTextures(1, &screenTexture);
    if (fbo) glDeleteFramebuffers(1, &fbo);
    if (shaderProgram) glDeleteProgram(shaderProgram);
}

/**
 * @brief Initializes the CRTEffect with the specified width and height.
 * Sets up the framebuffer, texture, shader program, and quad for rendering.
 * @param width The width of the texture.
 * @param height The height of the texture.
 */
bool CRTEffect::initialize(int width, int height) {
    // Setup FBO (Framebuffer Object)
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Create texture for the FBO
    glGenTextures(1, &screenTexture);                                                                   // Generate a texture to hold the rendered image
    glBindTexture(GL_TEXTURE_2D, screenTexture);                                                        // Bind the texture to the GL_TEXTURE_2D target                   
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);           // Allocate storage for the texture with specified width and height
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);                                   // Set the texture minification filter to linear            
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);                                   // Set the texture magnification filter to linear      
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);                                // Set the texture wrapping mode for the S coordinate to clamp to edge
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);                                // Set the texture wrapping mode for the T coordinate to clamp to edge
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);      // Attach the texture to the framebuffer as a color attachment

    // Check if the framebuffer is completely set up
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;                  // If it isn't complete, print an error message
        return false;                                                                                   // Return false to indicate failure, exit the function
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);                                                               // Unbind the framebuffer to avoid accidental modifications                         

    // Setup shader
    shaderProgram = ShaderManager::loadShader("shaders/crt.vs", "shaders/crt.frag");                    // Load the vertex and fragment shaders using the ShaderManager class
    if (!shaderProgram){
        std::cerr << "ERROR::SHADER:: Failed to load shaders!" << std::endl;                            // If shader loading fails, print an error message
        return false;                                                                                   // Return false to indicate failure, exit the function
    } 

    // Setup quad
    setupQuad();
    
    return true;                                                                                        // Return true to indicate successful initialization
}

/**
 * @brief Resizes the screen texture to the specified width and height.
 * @param width The new width of the texture.
 * @param height The new height of the texture.
 * This function updates the texture size to match the window dimensions.
 */
void CRTEffect::resize(int width, int height) {
    glBindTexture(GL_TEXTURE_2D, screenTexture);                                                        // Bind the texture to modify its properties -> was unbound in initialize()                 
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);           // Update the texture with new dimensions
}

/**
 * @brief Begins rendering to the framebuffer.
 * Clears the framebuffer with a specific color.
 */
void CRTEffect::beginRender() {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);                                                             // Bind the framebuffer to render to it                
    glClearColor(0.04f, 0.10f, 0.04f, 1.0f);                                                            // Set the clear color to a dark greenish tone                 
    glClear(GL_COLOR_BUFFER_BIT);                                                                       // Clear the framebuffer with the specified color                
}

/**
 * @brief Ends rendering to the framebuffer.
 * Binds the default framebuffer and clears the color buffer.
 */
void CRTEffect::endRender() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);                                                               // Unbind the framebuffer to return to the default framebuffer         
    glClear(GL_COLOR_BUFFER_BIT);                                                                       // Clear the color buffer of the default framebuffer                        
}

/**
 * @brief Renders the CRT effect using the shader program.
 * This function binds the shader program, sets the texture and time uniform, and draws a quad.
 * @param time The current time, used for animations in the shader.
 * This function is responsible for applying the CRT effect to the rendered scene.
 */
void CRTEffect::render(float time) {
    glUseProgram(shaderProgram);                                                                        // Use the shader program for rendering
    glActiveTexture(GL_TEXTURE0);                                                                       // Activate texture unit 0 -> this is where the screen texture will be bound               
    glBindTexture(GL_TEXTURE_2D, screenTexture);                                                        // Bind the screen texture to the active texture unit             
    glUniform1i(glGetUniformLocation(shaderProgram, "screenTexture"), 0);                               // Set the uniform for the screen texture in the shader program            
    glUniform1f(glGetUniformLocation(shaderProgram, "time"), time);                                     // Set the uniform for time in the shader program             

    glBindVertexArray(quadVAO);                                                                         // Bind the vertex array object for the quad          
    glDrawArrays(GL_TRIANGLES, 0, 6);                                                                   // Draw the quad using the vertex array object, specifying the number of vertices to draw                
    glBindVertexArray(0);                                                                               // Unbind the vertex array object to avoid accidental modifications                
}

/**
 * @brief Sets up a quad for rendering.
 * This function creates a vertex array object (VAO) and a vertex buffer object (VBO) for a quad that covers the entire screen.
 * Quad -> A rectangle that fills the screen, used for rendering the CRT effect.
 */
void CRTEffect::setupQuad() {
    // Define the vertices for a quad that covers the entire screen
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &quadVAO);                                                                     // Generate a vertex array object for the quad                                  
    glGenBuffers(1, &quadVBO);                                                                          // Generate a vertex buffer object for the quad                     
    glBindVertexArray(quadVAO);                                                                         // Bind the vertex array object to the GL_VERTEX_ARRAY target        
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);                                                             // Bind the vertex buffer object to the GL_ARRAY_BUFFER target                     
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);                 // Upload the vertex data to the GPU, specifying the size and usage pattern
    glEnableVertexAttribArray(0);                                                                       // Enable the vertex attribute at index 0 for position data                  
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);                       // Specify the layout of the vertex data for position (2 floats per vertex)
    glEnableVertexAttribArray(1);                                                                       // Enable the vertex attribute at index 1 for texture coordinates
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));     // Specify the layout of the vertex data for texture coordinates (2 floats per vertex)
    glBindVertexArray(0);                                                                               // Unbind the vertex array object to avoid accidental modifications              
}

/**
 * @brief Retrieves the width of the rendered screen texture.
 */
int CRTEffect::getWidth() {
    int width;
    glBindTexture(GL_TEXTURE_2D, screenTexture);                                                        // Bind the screen texture to retrieve its parameters          
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);                               // Get the width of the texture at level 0 
    return width;                                                                                       // Return the width of the texture        
}

/**
 * @brief Retrieves the height of the rendered screen texture.
 */
int CRTEffect::getHeight() {
    int height;
    glBindTexture(GL_TEXTURE_2D, screenTexture);                                                        // Bind the screen texture to retrieve its parameters          
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);                             // Get the height of the texture at level 0
    return height;                                                                                      // Return the height of the texture        
}