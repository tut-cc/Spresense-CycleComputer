/*
 * ファイル: InputManager.h
 * 説明: 入力ソース（ボタン）を管理し、抽象的な入力イベントを生成する。
 *       ボタンの押下および同時押しのロジックを処理する。
 */

#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include "../drivers/Button.h"
#include "../Config.h"

enum InputEvent {
    INPUT_NONE,
    INPUT_BTN_A,
    INPUT_BTN_B,
    INPUT_BTN_BOTH
};

class InputManager {
private:
    Button btnA;
    Button btnB;

public:
    InputManager() : btnA(BTN_A_PIN), btnB(BTN_B_PIN) {}

    void begin() {
        btnA.begin();
        btnB.begin();
    }

    InputEvent update() {
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
};

#endif
