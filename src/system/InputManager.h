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
    InputEvent update() {
        // Serial.println(INPUT_BTN_A);

        //この瞬間に、押されたか
        bool aPressed = btnA.isPressed();
        bool bPressed = btnB.isPressed();


        //同時押し、単押しの順番でやらないと、動きませんよー

        #ifdef DEBUGDAO

        // 同時押しの確認表示用
        if (aPressed && btnB.isHeld()){
             Serial.println("同時に押された");
             return INPUT_BTN_BOTH;
        }

        if (bPressed && btnA.isHeld()){
             Serial.println("同時に押された");
             return INPUT_BTN_BOTH;
        }
        #endif

        #ifdef IS_SPRESENSE
        // 単押しの確認用
        if (aPressed) {
            Serial.println("Aだよ");
            return INPUT_BTN_A;
        }
        if (bPressed) {
            Serial.println("Bだよ");
            return INPUT_BTN_B;
        }
        #endif

        // 同時押しの確認
        if (aPressed && btnB.isHeld()) return INPUT_BTN_BOTH;
        if (bPressed && btnA.isHeld()) return INPUT_BTN_BOTH;

        // 単押しの確認
        if (aPressed) return INPUT_BTN_A;
        if (bPressed) return INPUT_BTN_B;

        return INPUT_NONE;
    }
};
