#include <glad/glad.h>
#include "Font.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>
#include <glm/gtc/type_ptr.hpp>

/**
 * @brief Default constructor for Font.
 */
Font::Font() : VAO(0), VBO(0), shaderProgram(0) {}

/**
 * @brief Destructor for Font. Cleans up OpenGL resources and character textures.
 */
Font::~Font()
{
    for (auto &pair : characters)
    {
        if (pair.second.textureID)
            glDeleteTextures(1, &pair.second.textureID);
    }
    characters.clear();
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shaderProgram);
}

/**
 * @brief Loads a font from the specified file and prepares glyph textures for rendering.
 *
 * This function initializes the FreeType library, loads the font face from the given file path,
 * and generates OpenGL textures for the first 128 ASCII characters. It also sets up the necessary
 * OpenGL buffers and vertex array objects for rendering text. Any previously loaded character
 * textures are cleared before loading the new font.
 *
 * @param fontPath The file path to the font file (e.g., .ttf or .otf).
 * @param pixelHeight The desired pixel height for the loaded glyphs.
 * @return true if the font was loaded and prepared successfully, false otherwise.
 *
 * @note This function requires a valid OpenGL context and the FreeType library.
 * @note On failure, error messages are printed to std::cerr.
 */
bool Font::load(const std::string &fontPath, int pixelHeight)
{
    // clear old character textures
    for (auto &pair : characters)
    {
        if (pair.second.textureID)
            glDeleteTextures(1, &pair.second.textureID);
    }
    characters.clear();
    
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        std::cerr << "Failed to init FreeType library\n";
        return false;
    }

    FT_Face face;
    if (FT_New_Face(ft, fontPath.c_str(), 0, &face))
    {
        std::cerr << "Failed to load font: " << fontPath << "\n";
        FT_Done_FreeType(ft);
        return false;
    }

    FT_Set_Pixel_Sizes(face, 0, pixelHeight);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

    for (unsigned char c = 0; c < 128; c++)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cerr << "Failed to load glyph: " << (int)c << " '" << c << "'\n";
            continue;
        }

        GLuint tex = 0;
        if (face->glyph->bitmap.buffer)
        {
            glGenTextures(1, &tex);
            glBindTexture(GL_TEXTURE_2D, tex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
                         face->glyph->bitmap.width,
                         face->glyph->bitmap.rows,
                         0, GL_RED, GL_UNSIGNED_BYTE,
                         face->glyph->bitmap.buffer);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }

        Character character = {
            tex,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<GLuint>(face->glyph->advance.x)};

        characters.insert(std::make_pair(c, character));
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return initShader("shaders/font.vs.glsl", "shaders/font.fs.glsl");
}

/**
 * @brief Renders the given text string at the specified position, scale, and color.
 *
 * @param text The text string to render.
 * @param x The x-coordinate of the text's starting position.
 * @param y The y-coordinate of the text's baseline.
 * @param scale The scaling factor for the text size.
 * @param color The color of the text (RGB).
 */
void Font::renderText(const std::string &text, float x, float y, float scale, const glm::vec3 &color)
{
    glUseProgram(shaderProgram);
    glUniform3fv(glGetUniformLocation(shaderProgram, "textColor"), 1, glm::value_ptr(color));
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    for (char c : text)
    {
        if (c == '\t')
        {
            // Fallback to space if tab isn't available
            Character space = characters.count(' ') ? characters[' '] : Character{0, {0, 0}, {0, 0}, 10 << 6};
            x += (space.advance >> 6) * scale * 4; // Tab = 4 spaces
            continue;
        }

        if (characters.find(c) == characters.end())
            continue;

        Character ch = characters[c];

        if (isspace(c) && c != ' ') {
            x += (ch.advance >> 6) * scale;
            continue;
        }

        float xpos = x + ch.bearing.x * scale;
        float ypos = y - (ch.size.y - ch.bearing.y) * scale;

        float w = ch.size.x * scale;
        float h = ch.size.y * scale;

        if (w > 0 && h > 0)
        {
            float vertices[6][4] = {
                {xpos, ypos + h, 0.0f, 0.0f},
                {xpos, ypos, 0.0f, 1.0f},
                {xpos + w, ypos, 1.0f, 1.0f},
                {xpos, ypos + h, 0.0f, 0.0f},
                {xpos + w, ypos, 1.0f, 1.0f},
                {xpos + w, ypos + h, 1.0f, 0.0f}};
            glBindTexture(GL_TEXTURE_2D, ch.textureID);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        x += (ch.advance >> 6) * scale;
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

/**
 * @brief Initializes the shader program for font rendering.
 *
 * @param vertexPath Path to the vertex shader source file.
 * @param fragmentPath Path to the fragment shader source file.
 * @return true if the shader program was created successfully, false otherwise.
 */
bool Font::initShader(const std::string &vertexPath, const std::string &fragmentPath)
{
    std::string vertexCode = loadFileToString(vertexPath);
    std::string fragmentCode = loadFileToString(fragmentPath);

    const char *vShaderCode = vertexCode.c_str();
    const char *fShaderCode = fragmentCode.c_str();

    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, nullptr);
    glCompileShader(vertex);

    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, nullptr);
    glCompileShader(fragment);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertex);
    glAttachShader(shaderProgram, fragment);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return true;
}

/**
 * @brief Loads the contents of a file into a string.
 *
 * @param path The file path to load.
 * @return The contents of the file as a string.
 */
std::string Font::loadFileToString(const std::string &path)
{
    std::ifstream in(path);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

/**
 * @brief Calculates the width of the given text string when rendered at the specified scale.
 *
 * @param text The text string to measure.
 * @param scale The scaling factor for the text size.
 * @return The width of the text in pixels.
 */
float Font::getTextWidth(const std::string &text, float scale) const
{
    float width = 0.0f;
    for (char c : text)
    {
        auto it = characters.find(c);
        if (it != characters.end())
            width += (it->second.advance >> 6) * scale;
    }
    return width;
}

/**
 * @brief Sets the screen width for text rendering.
 *
 * @param width The width of the screen in pixels.
 */
void Font::setScreenWidth(int width)
{
    screenWidth = width;
}

/**
 * @brief Gets the current screen width used for text rendering.
 *
 * @return The width of the screen in pixels.
 */
int Font::getScreenWidth() const
{
    return screenWidth;
}