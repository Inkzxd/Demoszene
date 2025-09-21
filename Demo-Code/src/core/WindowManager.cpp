#include "WindowManager.h"
#include <iostream>

/**
 * @brief Static function to retrieve the WindowManager instance from a GLFW window.
 */
static WindowManager* getWindowManager(GLFWwindow* window) {
    return static_cast<WindowManager*>(glfwGetWindowUserPointer(window));
}

/**
 * @brief Key callback function to handle key events.
 * @param window The GLFW window that received the key event.
 * @param key The key that was pressed or released.
 * @param scancode The scancode of the key.
 * @param action The action performed (press, release, repeat).
 * @param mods Any modifier keys that were pressed (Shift, Ctrl, etc.).
 * Checks if a Key was pressed and calls the user-defined key callback function if set.
 */
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (auto* manager = getWindowManager(window)) {
        if (manager->keyCallbackFunc)
            manager->keyCallbackFunc(window, key, scancode, action, mods);
    }
}

/**
 * @brief Framebuffer size callback function to handle window resizing.
 * @param window The GLFW window that was resized.
 * @param width The new width of the framebuffer.
 * @param height The new height of the framebuffer.
 */
static void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    if (auto* manager = getWindowManager(window)) {
        if (manager->framebufferSizeCallbackFunc)
            manager->framebufferSizeCallbackFunc(window, width, height);
    }
}

/**
 * @brief Constructor for the WindowManager class.
 * Initializes GLFW and sets up the OpenGL context.
 */
WindowManager::WindowManager() : window(nullptr) {
    if (!glfwInit()) {                                                      // Initialize GLFW
        std::cerr << "Failed to initialize GLFW\n";                         // If GLFW initialization fails, print an error message and exit
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);                          // Set the OpenGL context version to 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);          // Use the core profile for OpenGL
    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);                // If the device uses macOS, set forward compatibility
    #endif
}

/**
 * @brief Destructor for the WindowManager class.
 * Cleans up the GLFW window and terminates GLFW.
 */
WindowManager::~WindowManager() {
    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}

/**
 * @brief Initializes the GLFW window with the specified parameters.
 * @param width The width of the window.
 * @param height The height of the window.
 * @param title The title of the window.
 * @param fullscreen Whether the window should be fullscreen or not.
 */
bool WindowManager::initialize(int width, int height, const char* title, bool fullscreen) {
    //GLFWmonitor* monitor = fullscreen ? glfwGetPrimaryMonitor() : nullptr;    // Get the primary monitor if fullscreen is requested
    window = glfwCreateWindow(width, height, title, nullptr, nullptr);          // Create a GLFW window with the specified width, height, and title

    if (!window) {
        std::cerr << "Failed to create GLFW window\n";                          // If window creation fails, print an error message and return false    
        return false;
    }

    glfwMakeContextCurrent(window);                                             // Make the created window the current OpenGL context
    glfwSetWindowUserPointer(window, this);                                     // Set user pointer for callbacks
    return true;
}

/**
 * @brief Sets the key callback function for handling key events.
 * @param callback The function to be called when a key event occurs.
 * This function allows the user to define custom behavior when keys are pressed, released, or repeated.
 */
void WindowManager::setKeyCallback(std::function<void(GLFWwindow*, int, int, int, int)> callback) {
    keyCallbackFunc = callback;
    glfwSetKeyCallback(window, keyCallback);
}

/**
 * @brief Sets the framebuffer size callback function for handling window resizing.
 * @param callback The function to be called when the framebuffer size changes.
 */
void WindowManager::setFramebufferSizeCallback(std::function<void(GLFWwindow*, int, int)> callback) {
    framebufferSizeCallbackFunc = callback;
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
}

/**
 * @brief Returns the GLFW window pointer.
 */
GLFWwindow* WindowManager::getWindow() const { 
    return window;
}

/**
 * @brief Retrieves the size of the framebuffer.
 * @param width Reference to an integer to store the width of the framebuffer.
 * @param height Reference to an integer to store the height of the framebuffer.
 */
void WindowManager::getFramebufferSize(int& width, int& height) const {
    glfwGetFramebufferSize(window, &width, &height);
}

/**
 * @brief Checks if the window should close.
 */
bool WindowManager::shouldClose() const {
    return glfwWindowShouldClose(window);
}

/**
 * @brief Swaps the front and back buffers of the window.
 */
void WindowManager::swapBuffers() const {
    glfwSwapBuffers(window);
}

/**
 * @brief Polls for events in the GLFW window.
 */
void WindowManager::pollEvents() const {
    glfwPollEvents();
}