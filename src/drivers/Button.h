/*
 * ファイル: Button.h
 * 説明: デバウンスロジック付きのボタン入力ドライバー。
 *       ボタンが押されたか、長押しされているかを確認するメソッドを提供します。
 */

#pragma once

#include <Arduino.h>

class Button {
   private:
    int pin;
    bool state;        // 入力ピンからの現在の安定した状態
    bool lastReading;  // 入力ピンからの前回の読み取り値
    unsigned long lastDebounceTime = 0;
    unsigned long debounceDelay = 50;  // 50ms 待機

   public:
    Button(int pin);

    void begin();

    // 立ち下がりエッジ（押下）の時のみ true を返す
    bool isPressed();

    bool isHeld();
        // スイッチの値が変更された場合、デバウンスタイマーをリセット
        if (reading != lastReading) {
            lastDebounceTime = millis();
        }

        if ((millis() - lastDebounceTime) > debounceDelay) {
            // 読み取り値が何であれ、デバウンス遅延よりも長くその状態にあるため、
            // それを実際の現在の状態として採用する:

            // ボタンの状態が変わった場合:
            if (reading != state) {
                state = reading;

                // 新しい状態が LOW (押下) の場合のみトグルする
                if (state == LOW) {
                    pressed = true;
                }
            }
        }

        lastReading = reading;
        return pressed;
    }

    //押しっぱなしの状態で十分反応する
    bool isHeld() {
        // Serial.print("stateは");
        // Serial.println(state);
        return (state == LOW);
    }
};
