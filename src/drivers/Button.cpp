#include "Button.h"

#include "../system/Logger.h"

Button::Button(int pin) : pinNumber(pin) {}

void Button::begin() {
    pinMode(pinNumber, INPUT_PULLUP);
    stablePinLevel = digitalRead(pinNumber);
    lastPinLevel = stablePinLevel;
    lastDebounceTime = millis();
    LOG_DEBUG_IF(Config::Debug::ENABLE_BUTTON_LOG, "Button initialized on pin %d. Initial state: %s", pinNumber, stablePinLevel == HIGH ? "HIGH" : "LOW");
}

bool Button::isPressed() {
    bool rawPinLevel = digitalRead(pinNumber);
    bool pressed = false;

    if (rawPinLevel != lastPinLevel) {
        resetDebounceTimer();
        LOG_DEBUG_IF(Config::Debug::ENABLE_BUTTON_LOG, "Pin %d raw change: %d -> %d", pinNumber, lastPinLevel, rawPinLevel);
    }

    if (hasDebounceTimePassed()) {
        if (stablePinLevel != rawPinLevel) {
            if (rawPinLevel == LOW) {
                pressed = true;
                LOG_DEBUG_IF(Config::Debug::ENABLE_BUTTON_LOG, "Button on pin %d PRESSED (stable)", pinNumber);
            } else {
                LOG_DEBUG_IF(Config::Debug::ENABLE_BUTTON_LOG, "Button on pin %d RELEASED (stable)", pinNumber);
            }
            stablePinLevel = rawPinLevel;
        }
    }

    lastPinLevel = rawPinLevel;
    return pressed;
}
