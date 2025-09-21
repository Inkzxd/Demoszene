#include "SoundManager.h"
#include <iostream>
#include <cstdlib>
#include <ctime>

/**
 * @brief Constructor for SoundManager class.
 * Initializes the random number generator seed.
 */
SoundManager::SoundManager() {
    std::srand(std::time(nullptr));
}

/**
 * @brief Destructor for SoundManager class.
 * Cleans up the sound buffers and sounds.
 */
SoundManager::~SoundManager() {
    // SFML objects will clean themselves up
}

/**
 * @brief Loads sound files into the sound manager.
 * @param soundFiles A vector of file paths to sound files.
 * @return True if all sounds are loaded successfully, false otherwise.
 * 
 * Shared sound buffer is used as otherwise the sound object would be destroyed when the function returns.
 * play-methods use shared sound buffers to ensure the sound is still when the methods are called.
 */
bool SoundManager::loadSounds(const std::vector<std::string>& soundFiles) {
    for (const auto& file : soundFiles) {                                   // Loop through each sound file provided
        auto sharedBuffer = std::make_shared<sf::SoundBuffer>();            // Create a shared pointer for the sound buffer
        if (!sharedBuffer->loadFromFile(file)) {                            // Attempt to load the sound file into the buffer
            std::cerr << "Failed to load sound file: " << file << "\n";     // If loading fails, print an error message
            return false;
        }
        buffers.push_back(sharedBuffer);                                    // Store the shared pointer in the buffers vector
        sf::Sound sound(*sharedBuffer);                                     // Create a sound object with the buffer
        sounds.push_back(sound);                                            // Add the sound object to the sounds vector    
    }
    return true;                                                            // Return true if all sounds are loaded successfully
}

/**
 * @brief Loads a single sound file into the sound manager.
 * @param soundFile The file path to the sound file.
 * @return True if the sound is loaded successfully, false otherwise.
 * 
 * This function creates a shared sound buffer and a sound object, then stores them in the respective vectors.
 */
bool SoundManager::loadSound(const std::string& soundFile) {
    auto sharedBuffer = std::make_shared<sf::SoundBuffer>();               
    if (!sharedBuffer->loadFromFile(soundFile)) {                         
        std::cerr << "Failed to load sound file: " << soundFile << "\n";    
        return false;
    }
    buffers.push_back(sharedBuffer);                                        
    sf::Sound sound(*sharedBuffer);                                         
    sounds.push_back(sound);                                                
    return true;                                                            
}

/**
 * @brief Plays a random sound from the loaded sounds.
 * If no sounds are loaded, this function does nothing.
 */
void SoundManager::playRandomSound() {
   int typingSoundCount = 5; // Number of typing sounds to choose from
    if (sounds.size() >= typingSoundCount) {
        int randomIndex = std::rand() % typingSoundCount;
        playSound(randomIndex);
    }
}

/**
 * @brief Plays a sound at the specified index.
 * @param index The index of the sound to play.
 */
void SoundManager::playSound(int index) {                                   
    if (index >= 0 && index < sounds.size()) {                              // Check if provided index is within the valid range
        sounds[index].play();                                               // If yes, play the sound at the specified index
    } else {
        std::cerr << "Sound index out of range: " << index << "\n";         // Otherwise, print an error message
    }
}

/**
 * @brief Loads a background sound from a file.
 * @param file The file path to the background sound.
 * @return True if the background sound is loaded successfully, false otherwise.
 */
bool SoundManager::loadBackgroundSound(const std::string& file) {
    if (!backgroundBuffer.loadFromFile(file)) {
        std::cerr << "Failed to load background sound file: " << file << "\n";
        return false;
    }
    backgroundSound.setBuffer(backgroundBuffer);
    backgroundSound.setLoop(true); // Optional: loop background sound
    return true;
}

/**
 * @brief Plays the loaded background sound.
 */
void SoundManager::playBackgroundSound() {
    backgroundSound.play();
}

void SoundManager::setBackgroundVolume(float volume) {
    if (volume < 0.0f || volume > 1.0f) {
        std::cerr << "Volume must be between 0.0f and 1.0f\n";
        return;
    }
    backgroundSound.setVolume(volume * 100.0f); // SFML uses a scale of 0-100 for volume
}

void SoundManager::stopBackgroundSound() {
    backgroundSound.stop(); // Stop the background sound
}