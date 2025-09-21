#include "ShaderManager.h"
#include <fstream>
#include <sstream>
#include <iostream>

/**
 * @brief Loads a shader program from vertex and fragment shader files.
 * @param vsPath Path to the vertex shader file.
 * @param fsPath Path to the fragment shader file.
 * @return The shader program ID.
 * This function reads the shader source code from the specified files, compiles the shaders, links them into a program, and returns the program ID.
 */
GLuint ShaderManager::loadShader(const char* vsPath, const char* fsPath) {
    std::ifstream vsFile(vsPath);                               // Open the vertex shader file                      
    std::stringstream vsStream;                                 // Create a string stream to read the file contents  
    vsStream << vsFile.rdbuf();                                 // Read the file contents into the string stream
    std::string vsCode = vsStream.str();                        // Convert the string stream to a string
    const char* vShaderCode = vsCode.c_str();                   // Get the C-style string from the vertex shader code

    std::ifstream fsFile(fsPath);                               // Open the fragment shader file, ff. same process as above
    std::stringstream fsStream;
    fsStream << fsFile.rdbuf();
    std::string fsCode = fsStream.str();
    const char* fShaderCode = fsCode.c_str();

    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);           // Create a vertex shader object
    glShaderSource(vertex, 1, &vShaderCode, nullptr);           // Set the source code for the vertex shader
    glCompileShader(vertex);

    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);       // Create a fragment shader object
    glShaderSource(fragment, 1, &fShaderCode, nullptr);         // Set the source code for the fragment shader
    glCompileShader(fragment);

    GLuint program = glCreateProgram();                         // Create a shader program object   
    glAttachShader(program, vertex);                            // Attach the vertex and fragment shaders to the program
    glAttachShader(program, fragment);
    glLinkProgram(program);                                     // Link the shaders into the program   

    glDeleteShader(vertex);                                     // Delete the vertex and fragment shader objects as they are no longer needed
    glDeleteShader(fragment);

    return program;                                             // Return the shader program ID
}

