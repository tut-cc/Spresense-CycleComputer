#include "Button.h"
#include "../Config.h"

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

    if (rawPinLevel != lastPinLevel) lastDebounceTime = millis();

    if ((millis() - lastDebounceTime) > Config::DEBOUNCE_DELAY) {
        if (rawPinLevel != stablePinLevel) {
            stablePinLevel = rawPinLevel;

            if (stablePinLevel == LOW) pressed = true;
        }
    }

    lastPinLevel = rawPinLevel;
    return pressed;
}

bool Button::isHeld() {
    return (stablePinLevel == LOW);
}
