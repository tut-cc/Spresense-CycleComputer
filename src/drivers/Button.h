#pragma once

class Button {
   private:
    int pinNumber;
    bool stablePinLevel;
    bool lastPinLevel;
    unsigned long lastDebounceTime;

   public:
    Button(int pin);
    void begin();
    bool isPressed();
    bool isHeld();
};
