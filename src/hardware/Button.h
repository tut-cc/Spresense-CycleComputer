#pragma once

#include <Arduino.h>

#include "../Config.h"

class Button {
private:
  const int     pinNumber;
  bool          stablePinLevel;
  bool          lastPinLevel;
  unsigned long lastDebounceTime;

public:
  virtual ~Button() {}
  Button(int pin) : pinNumber(pin) {}

  Button(const Button &)            = delete;
  Button &operator=(const Button &) = delete;

  virtual void begin() {
    pinMode(pinNumber, INPUT_PULLUP);
    stablePinLevel   = digitalRead(pinNumber);
    lastPinLevel     = stablePinLevel;
    lastDebounceTime = millis();
  }

  virtual bool isPressed() {
    bool rawPinLevel = digitalRead(pinNumber);
    bool pressed     = false;

    if (rawPinLevel != lastPinLevel) resetDebounceTimer();

    if (hasDebounceTimePassed()) {
      if (stablePinLevel != rawPinLevel) {
        if (rawPinLevel == LOW) pressed = true;
        stablePinLevel = rawPinLevel;
      }
    }

    lastPinLevel = rawPinLevel;
    return pressed;
  }

  virtual bool isHeld() const {
    return stablePinLevel == LOW;
  }

private:
  inline void resetDebounceTimer() {
    lastDebounceTime = millis();
  }

  inline bool hasDebounceTimePassed() const {
    return (millis() - lastDebounceTime) > Config::DEBOUNCE_DELAY;
  }
};
