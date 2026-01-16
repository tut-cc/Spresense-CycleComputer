#pragma once

#include <Arduino.h>

class Button {
private:
  const int                      pinNumber;
  bool                           stablePinLevel;
  bool                           lastPinLevel;
  unsigned long                  lastDebounceTime;
  bool                           pressed;
  static constexpr unsigned long DEBOUNCE_DELAY_MS = 50;

public:
  Button(int pin) : pinNumber(pin), pressed(false) {}

  void begin() {
    pinMode(pinNumber, INPUT_PULLUP);
    stablePinLevel   = digitalRead(pinNumber);
    lastPinLevel     = stablePinLevel;
    lastDebounceTime = millis();
    pressed          = false;
  }

  void update() {
    pressed                = false;
    const bool rawPinLevel = digitalRead(pinNumber);

    if (rawPinLevel != lastPinLevel) resetDebounceTimer();

    if (hasDebounceTimePassed()) {
      if (stablePinLevel != rawPinLevel) {
        stablePinLevel = rawPinLevel;
        if (stablePinLevel == LOW) pressed = true;
      }
    }

    lastPinLevel = rawPinLevel;
  }

  bool wasPressed() const {
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
    return DEBOUNCE_DELAY_MS < (millis() - lastDebounceTime);
  }
};
