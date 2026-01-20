#pragma once

/**
 * @file Button.h
 * @brief 物理ボタンのデバウンス処理と状態管理
 *
 * チャタリング防止のため、ステートマシンによる
 * デバウンス処理を実装しています。
 */

#include "../common/Config.h"
#include <Arduino.h>

class Button {
public:
  /// ボタンの状態を表す列挙型
  enum class State {
    High,             // ボタン離れている
    WaitStablizeHigh, // HIGH安定待ち
    Low,              // ボタン押されている
    WaitStablizeLow   // LOW安定待ち
  };

  const int     pinNumber;
  State         state;
  unsigned long lastStateChangeTime;
  bool          pressed;
  bool          held;

public:
  Button(int pin) : pinNumber(pin), state(State::High), pressed(false), held(false) {}

  void begin() {
    pinMode(pinNumber, INPUT_PULLUP);
    state   = (digitalRead(pinNumber) == LOW) ? State::Low : State::High;
    pressed = false;
  }

  void update() {
    pressed                         = false;
    const bool          rawPinLevel = digitalRead(pinNumber);
    const unsigned long now         = millis();

    switch (state) {
    case State::High:
      if (rawPinLevel == LOW) changeState(State::WaitStablizeLow, now);
      break;

    case State::WaitStablizeLow:
      if (rawPinLevel == HIGH) changeState(State::High, now);
      else if (now - lastStateChangeTime > Config::Button::DEBOUNCE_MS) {
        changeState(State::Low, now);
        pressed = true;
      }
      break;

    case State::Low:
      if (rawPinLevel == HIGH) changeState(State::WaitStablizeHigh, now);
      break;

    case State::WaitStablizeHigh:
      if (rawPinLevel == LOW) changeState(State::Low, now);
      else if (now - lastStateChangeTime > Config::Button::DEBOUNCE_MS)
        changeState(State::High, now);
      break;
    }
    held = (state == State::Low || state == State::WaitStablizeHigh);
  }

private:
  void changeState(State newState, unsigned long now) {
    state               = newState;
    lastStateChangeTime = now;
  }
};
