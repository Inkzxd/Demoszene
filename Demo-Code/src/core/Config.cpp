#include "Config.h"
#include "inih/ini.h"
#include <iostream>
#include <filesystem>

int Config::width = 1920;  // Default values
int Config::height = 1080;
bool Config::fullscreen = false;


/**
 * @brief INI file handler callback for configuration parsing.
 *
 * This function is called for each key-value pair parsed from the configuration file.
 * It updates the corresponding fields in the Config class based on the section and name.
 *
 * @param user Pointer to user data (unused).
 * @param section The section name in the INI file (e.g., "Window").
 * @param name The key name within the section (e.g., "Width", "Height", "Fullscreen").
 * @param value The value associated with the key as a string.
 * @return Always returns 1 to indicate success.
 */
static int handler(void* user, const char* section, const char* name, const char* value) {
    if (strcmp(section, "Window") == 0) {
        if (strcmp(name, "Width") == 0) {
            Config::width = atoi(value);
        } else if (strcmp(name, "Height") == 0) {
            Config::height = atoi(value);
        } else if (strcmp(name, "Fullscreen") == 0) {
            Config::fullscreen = atoi(value) != 0;
        }
    }
    return 1;
}

/**
 * @brief Loads configuration settings from the specified file.
 *
 * This function attempts to locate and parse a configuration file given by the filename.
 * It first checks if the file exists at the provided path. If the file does not exist,
 * an error message is printed and the function returns false. If the file exists,
 * it attempts to parse the file using the ini_parse function. If parsing fails,
 * an error message is printed and the function returns false. On success, the function returns true.
 *
 * @param filename The path to the configuration file to load.
 * @return true if the configuration file was found and successfully parsed; false otherwise.
 */
bool Config::load(const std::string& filename) {
    std::cout << "Looking for config at: " << std::filesystem::absolute(filename) << std::endl;
    
    if (!std::filesystem::exists(filename)) {
        std::cerr << "Config file does not exist at: " << std::filesystem::absolute(filename) << std::endl;
        return false;
    }
    
    if (ini_parse(filename.c_str(), handler, nullptr) < 0) {
        std::cerr << "Error parsing config file" << std::endl;
        return false;
    }
    return true;
}