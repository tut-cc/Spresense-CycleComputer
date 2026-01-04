#include "InputManager.h"

#include "../Config.h"

InputManager::InputManager() : btnA(BTN_A_PIN), btnB(BTN_B_PIN) {}

void InputManager::begin() {
    btnA.begin();
    btnB.begin();
}

InputEvent InputManager::update() {
    bool aPressed = btnA.isPressed();
    bool bPressed = btnB.isPressed();

    // 同時押しの確認
    if (aPressed && btnB.isHeld()) return INPUT_BTN_BOTH;
    if (bPressed && btnA.isHeld()) return INPUT_BTN_BOTH;

    // 単押しの確認
    if (aPressed) return INPUT_BTN_A;
    if (bPressed) return INPUT_BTN_B;

    return INPUT_NONE;
}
