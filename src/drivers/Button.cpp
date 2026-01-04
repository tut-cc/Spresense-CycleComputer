#include "Button.h"

Button::Button(int pin) : pin(pin) {}

void Button::begin() {
    pinMode(pin, INPUT_PULLUP);
    state = digitalRead(pin);
    lastReading = state;
}

// 立ち下がりエッジ（押下）の時のみ true を返す
bool Button::isPressed() {
    bool reading = digitalRead(pin);
    bool pressed = false;

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

bool Button::isHeld() {
    return (state == LOW);
}