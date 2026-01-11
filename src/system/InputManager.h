#pragma once

#include "../drivers/ButtonDriver.h"

#include "InputEvent.h"

namespace application {

class InputManager {
private:
  drivers::ButtonDriver btnA;
  drivers::ButtonDriver btnB;

public:
  InputManager() : btnA(Config::Pin::BTN_A), btnB(Config::Pin::BTN_B) {}

  void begin() {
    btnA.begin();
    btnB.begin();
  }

  InputEvent update() {
    bool aPressed = btnA.isPressed();
    bool bPressed = btnB.isPressed();

    if ((aPressed && btnB.isHeld()) || (bPressed && btnA.isHeld())) return InputEvent::BTN_BOTH;
    if (aPressed) return InputEvent::BTN_A;
    if (bPressed) return InputEvent::BTN_B;

    return InputEvent::NONE;
  }
};

} // namespace application
