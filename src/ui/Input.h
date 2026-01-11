#pragma once

#include "../drivers/Button.h"

#include "InputEvent.h"

namespace ui {

class Input {
private:
  drivers::Button btnA;
  drivers::Button btnB;

public:
  Input() : btnA(Config::Pin::BTN_A), btnB(Config::Pin::BTN_B) {}

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

} // namespace ui
