#include "Button.h"

Button::Button(int pin) : pin(pin) {}

void Button::begin() {
    pinMode(pin, INPUT_PULLUP);
    state = digitalRead(pin);
    lastReading = state;
}

bool Button::isPressed() {
    bool reading = digitalRead(pin);
    bool pressed = false;

    if (reading != lastReading) lastDebounceTime = millis();

    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (reading != state) {
            state = reading;

            if (state == LOW) pressed = true;
        }
    }

    lastReading = reading;
    return pressed;
}

bool Button::isHeld() {
    return (state == LOW);
}
