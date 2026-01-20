#pragma once

#include <Arduino.h>

constexpr unsigned long DEBOUNCE_DELAY_MS = 20;

class Button {
public:
  enum class State { High, WaitStablizeHigh, Low, WaitStablizeLow };

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
      else if (now - lastStateChangeTime > DEBOUNCE_DELAY_MS) {
        changeState(State::Low, now);
        pressed = true;
      }
      break;

    case State::Low:
      if (rawPinLevel == HIGH) changeState(State::WaitStablizeHigh, now);
      break;

    case State::WaitStablizeHigh:
      if (rawPinLevel == LOW) changeState(State::Low, now);
      else if (now - lastStateChangeTime > DEBOUNCE_DELAY_MS) changeState(State::High, now);
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
