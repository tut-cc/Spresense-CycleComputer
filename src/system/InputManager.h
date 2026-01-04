/*
 * ファイル: InputManager.h
 * 説明: 入力ソース（ボタン）を管理し、抽象的な入力イベントを生成する。
 *       ボタンの押下および同時押しのロジックを処理する。
 */
//入力管理といってもボタンだけです


#pragma once

#include "../drivers/Button.h"

enum InputEvent { INPUT_BTN_A=PIN_D00, INPUT_BTN_B=PIN_D01, INPUT_BTN_BOTH ,INPUT_NONE,  };

class InputManager {
   private:
    Button btnA;
    Button btnB;

   public:
    InputManager();

    void begin();

    //毎フレーム呼ばれてはいる
    InputEvent update();
};
