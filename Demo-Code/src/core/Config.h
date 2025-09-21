#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <cstring>
#include <glm/glm.hpp>


/**
 * @class Config
 * @brief Provides static configuration settings for the application.
 *
 * The Config class manages global configuration parameters such as
 * screen width, height, and fullscreen mode. It also provides a method
 * to load these settings from a file.
 *
 * @note All members are static and should be accessed directly via the class.
 */
class Config {
public:
    static bool load(const std::string& filename);

    static int width;
    static int height;
    static bool fullscreen;
};

#endif // CONFIG_H