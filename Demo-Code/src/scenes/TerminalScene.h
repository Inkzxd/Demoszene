#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <functional>
#include <numeric> // For std::accumulate

#include "graphics/Font.h"
#include "audio/SoundManager.h"

/**
 * @brief Class for creating a terminal animation effect.
 * This class handles the animation of text in a terminal-like style, including typing effects for directories and files, as well as displaying demo code output.
 * 
 * Implemented in TerminalAnimation.cpp.
 */
class TerminalScene {
public:
    struct CodeDemo {
        std::vector<std::string> code;
        std::vector<std::string> output;
    };

    TerminalScene();
    
    void setOnTypeCallback(std::function<void()> callback) { onTypeCallback = callback; }
    void update(float currentTime);
    void render(Font& font, float y, float lineSpacing, float currentTime, const glm::vec3& textColor);
    void reset();
    bool isFinished() const;

private:
    std::function<void()> onTypeCallback;
    std::string animationTextDirectory;
    std::string animationTextFile;
    std::string displayedTextDirectory;
    std::string displayedTextFile;
    size_t animationIndex;
    size_t animationIndexFile;
    size_t codeCharIndex;
    size_t directoryCharIndex;
    size_t fileCharIndex;
    double animationLastTime;
    float fileTypingStartTime;
    
    std::vector<CodeDemo> demos;
    size_t codeLineIndex;
    double lastCodeLineTime;
    double codeLineInterval;
    
    bool demoStarted;
    double demoStartTime;
    SoundManager soundManager;
    bool finished;

    void initializeDemos();

    mutable float currentTime = 0.0f;
};
