#include "ButtonDriver.h"

namespace drivers {

ButtonDriver::ButtonDriver(int pin) : pinNumber(pin) {}

void ButtonDriver::begin() {
  pinMode(pinNumber, INPUT_PULLUP);
  stablePinLevel = digitalRead(pinNumber);
  lastPinLevel = stablePinLevel;
  lastDebounceTime = millis();
}

bool ButtonDriver::isPressed() {
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
void ButtonDriver::setMockState(int pin, int state) {
  setPinState(pin, state);
}
#endif

} // namespace drivers
