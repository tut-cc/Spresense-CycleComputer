#include "InputManager.h"
#include "Logger.h"
#include "../Config.h"

InputManager::InputManager() : btnA(Config::Pin::BTN_A), btnB(Config::Pin::BTN_B) {}

void InputManager::begin() {
    btnA.begin();
    btnB.begin();
}

InputEvent InputManager::update() {
    // ボタンの押下状態を取得
    bool aPressed = btnA.isPressed();
    bool bPressed = btnB.isPressed();

    // 同時押しの確認
    if ((aPressed && btnB.isHeld()) || (bPressed && btnA.isHeld())) {
        Logger::logln("[InputManager] 同時に押された");
        return InputEvent::BTN_BOTH;
    }

    // 単押しの確認
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
