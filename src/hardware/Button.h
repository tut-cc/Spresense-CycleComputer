#pragma once

#include <Arduino.h>

constexpr unsigned long DEBOUNCE_DELAY_MS = 50;

class Button {
public:
  enum class State { High, WaitStablizeHigh, Low, WaitStablizeLow };

private:
  const int     pinNumber;
  State         state;
  unsigned long lastStateChangeTime;
  bool          pressEdge;

public:
  Button(int pin) : pinNumber(pin), state(State::High), pressEdge(false) {}

  void begin() {
    pinMode(pinNumber, INPUT_PULLUP);
    state     = (digitalRead(pinNumber) == LOW) ? State::Low : State::High;
    pressEdge = false;
  }

  void update() {
    pressEdge                       = false;
    const bool          rawPinLevel = digitalRead(pinNumber);
    const unsigned long now         = millis();

    switch (state) {
    case State::High: // 押されていない状態
      if (rawPinLevel == LOW) changeState(State::WaitStablizeLow, now);
      break;

    case State::WaitStablizeLow: // 押されていない->押されている？
      if (rawPinLevel == HIGH) changeState(State::High, now);
      else if (now - lastStateChangeTime > DEBOUNCE_DELAY_MS) {
        changeState(State::Low, now);
        pressEdge = true;
      }
      break;

    case State::Low: // 押されている状態
      if (rawPinLevel == HIGH) changeState(State::WaitStablizeHigh, now);
      break;

    case State::WaitStablizeHigh: // 押されている->押されていない？
      if (rawPinLevel == LOW) changeState(State::Low, now);
      else if (now - lastStateChangeTime > DEBOUNCE_DELAY_MS) changeState(State::High, now);
      break;
    }
  }

  bool isPressed() const {
    return pressEdge;
  }

  bool isHeld() const {
    return (state == State::Low || state == State::WaitStablizeHigh);
  }

private:
  void changeState(State newState, unsigned long now) {
    state               = newState;
    lastStateChangeTime = now;
  }
};
