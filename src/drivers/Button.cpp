#include "Button.h"

Button::Button(int pin) : pinNumber(pin) {}

void Button::begin() {
  pinMode(pinNumber, INPUT_PULLUP);
  stablePinLevel = digitalRead(pinNumber);
  lastPinLevel = stablePinLevel;
  lastDebounceTime = millis();
}

bool Button::isPressed() {
  bool rawPinLevel = digitalRead(pinNumber);
  bool pressed = false;

  if (rawPinLevel != lastPinLevel)
    resetDebounceTimer();

  if (hasDebounceTimePassed()) {
    if (stablePinLevel != rawPinLevel) {
      if (rawPinLevel == LOW)
        pressed = true;
      stablePinLevel = rawPinLevel;
    }
  }

  lastPinLevel = rawPinLevel;
  return pressed;
}
