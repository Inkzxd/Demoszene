#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include <SFML/Audio.hpp>
#include <vector>
#include <memory>
#include <string>

/**
 * @brief Class for managing sound effects in the application.
 * This class handles loading sound files, playing random sounds, and playing specific sounds by index.
 * Also includes constructors and destructors for resource management.
 * 
 * Implemented in SoundManager.cpp.
 */
class SoundManager {
public:
    SoundManager();
    ~SoundManager();
    
    bool loadSounds(const std::vector<std::string>& soundFiles);
    bool loadSound(const std::string& soundFile); // Load a single sound file
    bool loadBackgroundSound(const std::string& soundFile);
    void playRandomSound();
    void playSound(int index);
    void playBackgroundSound();
    void setBackgroundVolume(float volume); // Set volume for background sound (0.0f = mute, 1.0f = max)
    void stopBackgroundSound(); // Stop the background sound

private:
    std::vector<std::shared_ptr<sf::SoundBuffer>> buffers;
    std::vector<sf::Sound> sounds;

    sf::SoundBuffer backgroundBuffer; // Buffer for background sound
    sf::Sound backgroundSound;         // Sound object for background sound
};

#endif // SOUNDMANAGER_H