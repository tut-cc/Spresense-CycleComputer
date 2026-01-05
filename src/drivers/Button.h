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
    bool isPressed();
    bool isHeld();
};
