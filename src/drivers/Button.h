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

    //押しっぱなしの状態で十分反応する
    bool isHeld();
};
