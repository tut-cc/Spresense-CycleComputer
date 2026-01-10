#pragma once

#include "../Config.h"
#include "Arduino.h"

class Button {
   private:
    const int pinNumber;
    bool stablePinLevel;
    bool lastPinLevel;
    unsigned long lastDebounceTime;

    inline void resetDebounceTimer() {
        lastDebounceTime = millis();
    }

    inline bool hasDebounceTimePassed() const {
        return (millis() - lastDebounceTime) > Config::DEBOUNCE_DELAY;
    }

   public:
    Button(int pin);
    void begin();
    bool isPressed();

    inline bool isHeld() const {
        return stablePinLevel == LOW;
    }
};
