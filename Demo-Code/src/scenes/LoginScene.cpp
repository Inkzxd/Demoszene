#include "LoginScene.h"
#include <algorithm>
#include <vector>

/**
 * @brief Constructor. Initializes the login scene state.
 */
LoginScene::LoginScene()
    : stage(SHOW_USERNAME),
      username("htw saar"),
      password("********"),
      typedUsername(""),
      typedPassword(""),
      timer(0.0f),
      charIndex(0),
      verifyingDots(0.0f),
      accessTimer(0.0f)
{}

/**
 * @brief Update the login scene animation and state.
 * @param deltaTime Time elapsed since last update.
 */
void LoginScene::update(float deltaTime) {
    timer += deltaTime;
    switch(stage) {
        case SHOW_USERNAME:
            if (charIndex < (int)username.length() && timer > 0.18f) {
                typedUsername += username[charIndex++];
                timer = 0.0f;
                if (onTypeCallback) {
                    onTypeCallback(); // Play typing sound
                }
            }
            if (charIndex == (int)username.length()) {
                stage = WAIT_PASSWORD; 
                timer = 0.0f;
            }
            break;
        case WAIT_PASSWORD:
            if (timer > 0.5f) { // Wait for 0.5 seconds before showing password
                stage = SHOW_PASSWORD;
                timer = 0.0f;
                charIndex = 0; // Reset character index for password
            }
            break;
        case SHOW_PASSWORD:
            if (charIndex < (int)password.length() && timer > 0.18f) {
                typedPassword += password[charIndex++];
                timer = 0.0f;
                if (onTypeCallback) {
                    onTypeCallback(); // Play typing sound
                }
            }
            if (charIndex == (int)password.length()) {
                stage = VERIFYING;
                timer = 0.0f;
                verifyingDots = 0.0f;
            }
            break;
        case VERIFYING:
            verifyingDots += deltaTime * 2.0f;
            if (timer > 1.5f) {
                stage = ACCESS_GRANTED;
                timer = 0.0f;
                accessTimer = 0.0f;
            }
            break;
        case ACCESS_GRANTED:
            accessTimer += deltaTime;
            if (accessTimer > 1.5f) {
                stage = FINISHED;
            }
            break;
        case FINISHED:
            // nothing
            break;
    }
}

/**
 * @brief Render the login scene, including username, password, and status messages.
 * @param font Font object for text rendering.
 * @param y Y position for text.
 * @param lineSpacing Line spacing for text.
 * @param time Current time.
 * @param color Text color.
 */
void LoginScene::render(Font& font, float y, float lineSpacing, float time, const glm::vec3& color) {
    bool promptCursor = int(time * 8) % 2 == 0;

    float startY = y;

    // LOGIN label (zentriert)
    std::string loginLabel = "LOGIN:";
    float loginX = (font.getScreenWidth() - font.getTextWidth(loginLabel, 1.0f)) / 2.0f;
    font.renderText(loginLabel, loginX, startY, 1.0f, color);

    // Username-Zeile (Label an festem Platz, Animation rechts daneben)
    float unameY = startY - lineSpacing;
    std::string unameLabel = "Username: ";
    std::string unameAnim = (stage == SHOW_USERNAME) ? typedUsername + (promptCursor ? "_" : "") : username;
    float unameLabelWidth = font.getTextWidth(unameLabel, 1.0f);
    float unameLabelX = (font.getScreenWidth() - unameLabelWidth - 112) / 2.0f;
    font.renderText(unameLabel, unameLabelX, unameY, 1.0f, color);
    // Animierter Text immer direkt rechts daneben
    font.renderText(unameAnim, unameLabelX + unameLabelWidth, unameY, 1.0f, color);

    // Password-Zeile (Label an festem Platz, Animation rechts daneben)
    float pwdY = unameY - lineSpacing;
    std::string pwdLabel = "Password: ";
    std::string pwdAnim;
    if (stage == SHOW_PASSWORD) {
        pwdAnim = typedPassword + (promptCursor ? "_" : "");
    } else if (stage > SHOW_PASSWORD) {
        pwdAnim = password;
    }
    float pwdLabelWidth = font.getTextWidth(pwdLabel, 1.0f);
    float pwdLabelX = (font.getScreenWidth() - pwdLabelWidth - 112) / 2.0f;
    font.renderText(pwdLabel, pwdLabelX, pwdY, 1.0f, color);
    font.renderText(pwdAnim, pwdLabelX + pwdLabelWidth, pwdY, 1.0f, color);

    // Verifying-Animation (wie gehabt)
    if (stage == VERIFYING) {
        int dots = int(verifyingDots) % 4;
        std::string verifying = "Verifying" + std::string(dots, '.');
        float verifyingX = (font.getScreenWidth() - font.getTextWidth(verifying, 1.0f)) / 2.0f;
        font.renderText(verifying, verifyingX, pwdY - lineSpacing, 1.0f, color);
    }

    // ACCESS GRANTED (wie gehabt)
    if (stage == ACCESS_GRANTED || stage == FINISHED) {
        std::string granted = "ACCESS GRANTED";
        float grantedX = (font.getScreenWidth() - font.getTextWidth(granted, 1.2f)) / 2.0f;
        font.renderText(granted, grantedX, pwdY - 2 * lineSpacing + 12.0f, 1.2f, glm::vec3(0.0f, 1.0f, 0.0f));
    }
}

/**
 * @brief Reset the login scene to its initial state.
 */
void LoginScene::reset() {
    stage = SHOW_USERNAME;
    username = "htw saar";
    password = "********";
    typedUsername.clear();
    typedPassword.clear();
    timer = 0.0f;
    charIndex = 0;
    verifyingDots = 0.0f;
    accessTimer = 0.0f;
}
