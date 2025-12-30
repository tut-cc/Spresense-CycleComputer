/*
 * ファイル: Button.h
 * 説明: デバウンスロジック付きのボタン入力ドライバー。
 *       ボタンが押されたか、長押しされているかを確認するメソッドを提供します。
 */

#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

class Button {
   private:
    int _pin;
    bool _state;        // 入力ピンからの現在の安定した状態
    bool _lastReading;  // 入力ピンからの前回の読み取り値
    unsigned long _lastDebounceTime = 0;
    unsigned long _debounceDelay = 50;  // 50ms 待機

   public:
    Button(int pin) : _pin(pin) {}

    void begin() {
        pinMode(_pin, INPUT_PULLUP);
        _state = digitalRead(_pin);
        _lastReading = _state;
    }

    // 立ち下がりエッジ（押下）の時のみ true を返す
    bool isPressed() {
        bool reading = digitalRead(_pin);
        bool pressed = false;

        // スイッチの値が変更された場合、デバウンスタイマーをリセット
        if (reading != _lastReading) {
            _lastDebounceTime = millis();
        }

        if ((millis() - _lastDebounceTime) > _debounceDelay) {
            // 読み取り値が何であれ、デバウンス遅延よりも長くその状態にあるため、
            // それを実際の現在の状態として採用する:

            // ボタンの状態が変わった場合:
            if (reading != _state) {
                _state = reading;

                // 新しい状態が LOW (押下) の場合のみトグルする
                if (_state == LOW) {
                    pressed = true;
                }
            }
        }

        _lastReading = reading;
        return pressed;
    }

    bool isHeld() {
        return (_state == LOW);
    }
};

#endif
