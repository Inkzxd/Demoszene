#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <GLFW/glfw3.h>
#include <functional>

/**
 * @brief Class for managing a GLFW window.
 * This class handles the creation, initialization, and management of a GLFW window, including setting up callbacks for key events and framebuffer size changes.
 * 
 * Implemented in WindowManager.cpp.
 */
class WindowManager {
public:
    WindowManager();
    ~WindowManager();
    
    bool initialize(int width, int height, const char* title, bool fullscreen = false);
    void setKeyCallback(std::function<void(GLFWwindow*, int, int, int, int)> callback);
    void setFramebufferSizeCallback(std::function<void(GLFWwindow*, int, int)> callback);
    
    GLFWwindow* getWindow() const;
    void getFramebufferSize(int& width, int& height) const;
    bool shouldClose() const;
    void swapBuffers() const;
    void pollEvents() const;

    std::function<void(GLFWwindow*, int, int, int, int)> keyCallbackFunc;
    std::function<void(GLFWwindow*, int, int)> framebufferSizeCallbackFunc;

private:
    GLFWwindow* window;
};

#endif // WINDOWMANAGER_H