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
    if (aPressed && btnB.isHeld()) {
#ifdef DEBUGDAO
        Serial.println("[InputManager] 同時に押されたァ");
#endif
        return INPUT_BTN_BOTH;
    }

    if (bPressed && btnA.isHeld()) {
#ifdef DEBUGDAO
        Serial.println("[InputManager] 同時に押されたァ");
#endif
        return INPUT_BTN_BOTH;
    }

    // 単押しの確認
    if (aPressed) {
#ifdef DEBUGDAO
        Serial.println("[InputManager] Aボタンが押された");
#endif
        return INPUT_BTN_A;
    }

    if (bPressed) {
#ifdef DEBUGDAO
        Serial.println("[InputManager] Bボタンが押された");
#endif
        return INPUT_BTN_B;
    }

    return INPUT_NONE;
}
