/*
 * ファイル: InputManager.h
 * 説明: 入力ソース（ボタン）を管理し、抽象的な入力イベントを生成する。
 *       ボタンの押下および同時押しのロジックを処理する。
 */

#pragma once

#include "../drivers/Button.h"

enum InputEvent { INPUT_NONE, INPUT_BTN_A, INPUT_BTN_B, INPUT_BTN_BOTH };

class InputManager {
   private:
    Button btnA;
    Button btnB;

   public:
    InputManager();

    void begin();

    InputEvent update();
};
