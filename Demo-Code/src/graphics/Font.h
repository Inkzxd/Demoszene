#ifndef FONT_H
#define FONT_H

#include <GLFW/glfw3.h>
#include <ft2build.h>

#include <glm/glm.hpp>
#include <map>
#include <string>
#include FT_FREETYPE_H

/**
 * @brief Stores all relevant information for a single character glyph.
 */
struct Character {
    GLuint textureID;    ///< OpenGL texture ID for the glyph.
    glm::ivec2 size;     ///< Size of the glyph in pixels.
    glm::ivec2 bearing;  ///< Offset from baseline to left/top of glyph.
    GLuint advance;      ///< Offset to advance to next glyph.
};

/**
 * @brief Font rendering class using FreeType and OpenGL.
 *
 * Provides functionality to load fonts, render text, and manage font-related OpenGL resources.
 */
class Font {
   public:
    /**
     * @brief Constructs a Font object.
     */
    Font();

    /**
     * @brief Destructor. Cleans up OpenGL and font resources.
     */
    ~Font();

    /**
     * @brief Loads a font from file and prepares glyph textures.
     * @param fontPath Path to the font file (e.g., .ttf).
     * @param pixelHeight Desired pixel height for glyphs.
     * @return True if loading was successful, false otherwise.
     */
    bool load(const std::string& fontPath, int pixelHeight);

    /**
     * @brief Renders a text string at the specified position, scale, and color.
     * @param text The text to render.
     * @param x X position.
     * @param y Y position (baseline).
     * @param scale Scaling factor.
     * @param color Text color (RGB).
     */
    void renderText(const std::string& text, float x, float y, float scale, const glm::vec3& color);

    /**
     * @brief Returns the OpenGL shader program used for font rendering.
     * @return Shader program ID.
     */
    GLuint getShaderProgram() const { return shaderProgram; }

    /**
     * @brief Calculates the width of the given text string at the specified scale.
     * @param text The text to measure.
     * @param scale Scaling factor.
     * @return Width in pixels.
     */
    float getTextWidth(const std::string& text, float scale = 1.0f) const;

    /**
     * @brief Sets the screen width for text rendering.
     * @param width Screen width in pixels.
     */
    void setScreenWidth(int width);

    /**
     * @brief Gets the current screen width used for text rendering.
     * @return Screen width in pixels.
     */
    int getScreenWidth() const;

   private:
    std::map<char, Character> characters;
    GLuint VAO, VBO;
    GLuint shaderProgram;
    int screenWidth = 800;  // Default screen width

    bool initShader(const std::string& vertexPath, const std::string& fragmentPath);
    std::string loadFileToString(const std::string& path);
};

#endif  // FONT_H
