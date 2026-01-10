#include "Button.h"

namespace drivers {

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

#ifdef UNIT_TEST
void Button::resetMock() {
  // No static state to reset currently in Button class itself
  // Maybe reset Arduino mock pin states if we could access them?
  // For now do nothing or just rely on test setup to reset pins
}

void Button::setMockState(int pin, int state) {
  setPinState(pin, state);
}
#endif

} // namespace drivers
