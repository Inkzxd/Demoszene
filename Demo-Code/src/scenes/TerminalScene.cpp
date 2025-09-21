#include "TerminalScene.h"

/**
 * @brief Constructor for TerminalScene class.
 * Initializes the animation text, displayed text, and other member variables.
 * Sets up the initial state for the terminal animation.
 */
TerminalScene::TerminalScene() :
    animationTextDirectory("cd RetroTerminal/"),                    // Text to animate for directory
    animationTextFile("./RetroTerminal"),                           // Text to animate for file       
    displayedTextDirectory("user@retroterminal:~$ "),               // Text to be displayed for directory
    displayedTextFile("user@retroterminal:~RetroTerminal$ "),       // Text to be displayed for file
    animationIndex(0),                                              // Index for directory animation, used to track the current character being animated
    animationIndexFile(0),                                          // Index for file animation, used to track the current character being animated
    animationLastTime(0.0),                                         // Last time the animation was updated, used to control the typing speed    
    codeLineIndex(0),                                               // Index for code demo animation, used to track the current line of code being displayed    
    lastCodeLineTime(0.0),                                          // Last time a code line was displayed, used to control the typing speed of code lines
    codeLineInterval(0.5),                                          // Interval between displaying code lines, controls the speed of the code demo animation
    demoStarted(false),                                             // Flag to indicate if the demo has started, used to control when the code demo animation begins
    demoStartTime(0.0) {                                            // Start time for the demo, used to control the timing of the code demo animation
    fileTypingStartTime = -1.0f;                                    // Initialize file typing start time to -1, indicating no typing animation has started yet
    initializeDemos();                                              // Initialize the code demos with predefined code and output
}

/**
 * @brief Sets a callback function to be called when a character is typed in the terminal animation.
 */
void TerminalScene::update(float currentTime) {
    this->currentTime = currentTime; // Update the current time for the animation
    // Update directory typing animation
    if (animationIndex < animationTextDirectory.size() && currentTime - animationLastTime >= 0.1) {     // Check if enough time has passed to type the next character
        displayedTextDirectory += animationTextDirectory[animationIndex];                               // Append the next character from the animation text to the displayed text
        animationIndex++;                                                                               // Increment the animation index to move to the next character
        animationLastTime = currentTime;                                                                // Update the last time the animation was updated
        if (onTypeCallback) {
            onTypeCallback();                                                                           // Call the callback if set, indicating a character has been typed -> plays sound in main.cpp
        }
    }

    // Update file typing animation with 1 second delay after directory typing animation
    if (animationIndex >= animationTextDirectory.size() && animationIndexFile == 0 && fileTypingStartTime < 0.0f) {
        fileTypingStartTime = currentTime;
    }
    if (animationIndex >= animationTextDirectory.size() &&                                              // Check if directory typing animation is complete
        animationIndexFile < animationTextFile.size() &&
        fileTypingStartTime >= 0.0f &&
        currentTime - fileTypingStartTime >= 1.0f &&
        currentTime - animationLastTime >= 0.1) {
        displayedTextFile += animationTextFile[animationIndexFile];                                     // Append the next character from the file animation text to the displayed text
        animationIndexFile++;                                                                           // Increment the file animation index to move to the next character 
        animationLastTime = currentTime;                                                                // Update the last time the animation was updated
        if (onTypeCallback) {
            onTypeCallback();                                                                           // Call the callback if set, indicating a character has been typed -> plays sound in main.cpp
        }
    }
    // Reset fileTypingStartTime if animation is reset
    if (animationIndex == 0 && animationIndexFile == 0) {
        fileTypingStartTime = -1.0f;
    }

    // Start demo after typing animations complete
    if (animationIndex >= animationTextDirectory.size() &&
        animationIndexFile >= animationTextFile.size() &&
        !demoStarted) {
        demoStarted = true;
        demoStartTime = currentTime;
    }

    // Update code demo animation
    if (demoStarted && currentTime - demoStartTime > 0.5 && !demos.empty()) {
        const auto& codeLines = demos[0].code;
        if (codeLineIndex < codeLines.size()) {
            if (codeCharIndex < codeLines[codeLineIndex].size()) {
                if (currentTime - lastCodeLineTime > 0.05) { // Typing speed per character
                    codeCharIndex++;
                    lastCodeLineTime = currentTime;
                    if (onTypeCallback) {
                        onTypeCallback();
                    }
                }
            } else if (currentTime - lastCodeLineTime > codeLineInterval) {
                codeLineIndex++;
                codeCharIndex = 0;
                lastCodeLineTime = currentTime;
            }
        }
    }
}

/**
 * @brief Renders the terminal animation using the provided font, position, line spacing, current time, and text color.
 * @param font The font to use for rendering text.
 * @param y The vertical position to start rendering text.
 * @param lineSpacing The spacing between lines of text.
 * @param currentTime The current time in seconds, used for timing animations.
 * @param textColor The color of the text to render.
 * This function renders the initial prompt, directory typing animation, file typing animation, and demo code output in the terminal style.
 */
void TerminalScene::render(Font& font, float y, float lineSpacing, float currentTime, const glm::vec3& textColor) {
    // Render directory typing animation (jetzt an oberster Stelle)
    std::string dirLine = displayedTextDirectory;
    if (animationIndex < animationTextDirectory.size()) {
        // Blink every ~0.5 seconds
        if (static_cast<int>(currentTime * 8) % 2 == 0) {
            dirLine += "_"; // Show cursor while typing (blinking)
        }
    }
    font.renderText(dirLine, 10.0f, y, 1.0f, textColor);

    // Render file typing animation
    if (animationIndex >= animationTextDirectory.size()) {
        std::string fileLine = displayedTextFile;
        // Make the prompt cursor blink while typing
        if (animationIndexFile < animationTextFile.size()) {
            if (static_cast<int>(currentTime * 8) % 2 == 0) {
                fileLine += "_";
            }
        }
        font.renderText(fileLine, 10.0f, y - lineSpacing, 1.0f, textColor);
    }

    // Render demo code, one character at a time
    if (demoStarted && currentTime - demoStartTime > 0.5) {
        float code_y = y - 4 * lineSpacing;
        if (!demos.empty()) {
            font.renderText("Demo:", 10.0f, code_y, 1.0f, textColor);
            code_y -= lineSpacing;

            // Render code lines, one character at a time
            for (size_t i = 0; i < codeLineIndex && i < demos[0].code.size(); ++i) {
                font.renderText(demos[0].code[i], 30.0f, code_y, 1.0f, textColor);
                code_y -= lineSpacing;
            }

            // Render current line, one character at a time, with blinking cursor
            if (codeLineIndex < demos[0].code.size()) {
                const std::string& line = demos[0].code[codeLineIndex];
                std::string toRender = line.substr(0, codeCharIndex);
                // Add blinking cursor if still typing this line
                if (codeCharIndex < line.size() && static_cast<int>(currentTime * 2) % 2 == 0) {
                    toRender += "_";
                }
                font.renderText(toRender, 30.0f, code_y, 1.0f, textColor);
                code_y -= lineSpacing;
            }

            // If all code lines are fully rendered, show output
            if (codeLineIndex >= demos[0].code.size()) {
                code_y -= lineSpacing / 2;
                font.renderText("Output:", 10.0f, code_y, 1.0f, textColor);
                code_y -= lineSpacing;
                for (const auto& outLine : demos[0].output) {
                    font.renderText(outLine, 30.0f, code_y, 1.0f, textColor);
                    code_y -= lineSpacing;
                }
                if (!finished && currentTime - lastCodeLineTime > 1.0f) {
                    finished = true;
                }
            }
        }
    }
}

/**
 * @brief Resets the terminal animation to its initial state.
 * This function clears the displayed text, resets animation indices, and sets the demo state to not started.
 * It is used to restart the animation or when the terminal needs to be cleared.
 * 
 * Currently not in use
 */
void TerminalScene::reset() {
    displayedTextDirectory = "user@retroterminal:~$ ";
    displayedTextFile = "user@retroterminal:~RetroTerminal$ ";
    animationIndex = 0;
    animationIndexFile = 0;
    animationLastTime = 0.0;
    codeLineIndex = 0;
    codeCharIndex = 0;
    lastCodeLineTime = 0.0;
    demoStarted = false;
    demoStartTime = 0.0;
    finished = false;
    fileTypingStartTime = -1.0f;
}


/**
 * @brief Initializes the code demos with predefined code and output.
 * Sets the code demo thats displayed in the terminal animation on program start.
 */
void TerminalScene::initializeDemos() {
    demos = {
        {{"#include <iostream>",
          "",
          "int main() {",
          "    std::cout << \"Projektarbeit: Einfuehrung in die Demoszene\" << std::endl;",
          "    std::cout << \"Teilnehmer: Christian Petry, Xudong Zhang\" << std::endl;",
          "    return 0;",
          "}"},
         {"Projektarbeit: Einfuehrung in die Demoszene",
          "Teilnehmer: Christian Petry, Xudong Zhang"}}
    };
}

bool TerminalScene::isFinished() const {
    return animationIndex >= animationTextDirectory.size() &&
           animationIndexFile >= animationTextFile.size() &&
           demoStarted && 
           codeLineIndex >= demos[0].code.size() &&
           (currentTime - lastCodeLineTime > 2.0f);
}