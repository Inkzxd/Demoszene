#ifndef SHADERMANAGER_H
#define SHADERMANAGER_H

#include <glad/glad.h>
#include <string>

/**
 * @brief Class for managing shader loading and compilation.
 * This class provides a static method to load shaders from specified file paths, compile them, and link them into a shader program.
 * 
 * Implemented in ShaderManager.cpp.
 */
class ShaderManager {
public:
    static GLuint loadShader(const char* vsPath, const char* fsPath);
};

#endif // SHADERMANAGER_H