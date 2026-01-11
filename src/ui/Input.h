#pragma once

#include "InputEvent.h"

namespace ui {

template <typename ButtonT> class Input {
private:
  ButtonT &btnA;
  ButtonT &btnB;

public:
  Input(ButtonT &buttonA, ButtonT &buttonB) : btnA(buttonA), btnB(buttonB) {}

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
