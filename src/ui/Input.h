#pragma once

#include "../hardware/Button.h"
#include "InputEvent.h"

class Input {
private:
  Button &btnA;
  Button &btnB;

public:
  Input(Button &buttonA, Button &buttonB) : btnA(buttonA), btnB(buttonB) {}
  virtual ~Input() {}

  virtual void begin() {
    btnA.begin();
    btnB.begin();
  }

  virtual InputEvent update() {
    bool aPressed = btnA.isPressed();
    bool bPressed = btnB.isPressed();

    if ((aPressed && btnB.isHeld()) || (bPressed && btnA.isHeld())) return InputEvent::BTN_BOTH;
    if (aPressed) return InputEvent::BTN_A;
    if (bPressed) return InputEvent::BTN_B;

    return InputEvent::NONE;
  }
};
