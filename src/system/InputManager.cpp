#include "InputManager.h"

#include "../Config.h"
#include "Logger.h"

InputManager::InputManager() : btnA(Config::Pin::BTN_A), btnB(Config::Pin::BTN_B) {}

void InputManager::begin() {
    btnA.begin();
    btnB.begin();
}

InputEvent InputManager::update() {
    bool aPressed = btnA.isPressed();
    bool bPressed = btnB.isPressed();

    if ((aPressed && btnB.isHeld()) || (bPressed && btnA.isHeld())) {
        LOG_DEBUG("[InputManager] 同時に押された");
        return InputEvent::BTN_BOTH;
    }

    if (aPressed) {
        LOG_DEBUG("[InputManager] Aボタンが押された");
        return InputEvent::BTN_A;
    }

    if (bPressed) {
        LOG_DEBUG("[InputManager] Bボタンが押された");
        return InputEvent::BTN_B;
    }

    return InputEvent::NONE;
}
