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
  Button(int pin) : pinNumber(pin) {}

  void begin() {
    pinMode(pinNumber, INPUT_PULLUP);
    stablePinLevel   = digitalRead(pinNumber);
    lastPinLevel     = stablePinLevel;
    lastDebounceTime = millis();
  }

  bool isPressed() {
    const bool rawPinLevel = digitalRead(pinNumber);
    bool       pressed     = false;

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

  bool isHeld() const {
    return stablePinLevel == LOW;
  }

private:
  void resetDebounceTimer() {
    lastDebounceTime = millis();
  }

  bool hasDebounceTimePassed() const {
    return Config::DEBOUNCE_DELAY < (millis() - lastDebounceTime);
  }
};
