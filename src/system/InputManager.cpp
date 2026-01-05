#include "InputManager.h"

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
#ifdef DEBUG_MODE
        Serial.println("[InputManager] 同時に押されたァ");
#endif
        return InputEvent::BTN_BOTH;
    }

    // 単押しの確認
    if (aPressed) {
#ifdef DEBUG_MODE
        Serial.println("[InputManager] Aボタンが押された");
#endif
        return InputEvent::BTN_A;
    }

    if (bPressed) {
#ifdef DEBUG_MODE
        Serial.println("[InputManager] Bボタンが押された");
#endif
        return InputEvent::BTN_B;
    }

    return InputEvent::NONE;
}
