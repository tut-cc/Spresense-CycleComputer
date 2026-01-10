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
        Logger::logln("[InputManager] 同時に押された");
        return InputEvent::BTN_BOTH;
    }

    if (aPressed) {
        Logger::logln("[InputManager] Aボタンが押された");
        return InputEvent::BTN_A;
    }

    if (bPressed) {
        Logger::logln("[InputManager] Bボタンが押された");
        return InputEvent::BTN_B;
    }

    return InputEvent::NONE;
}
