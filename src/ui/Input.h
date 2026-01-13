#pragma once

#include "../hardware/Button.h"

class Input {
private:
  Button &btnA;
  Button &btnB;

public:
  enum class ID {
    NONE,
    BTN_A,
    BTN_B,
    BTN_BOTH,
  };

  Input(Button &buttonA, Button &buttonB) : btnA(buttonA), btnB(buttonB) {}
  virtual ~Input() {}

  virtual void begin() {
    btnA.begin();
    btnB.begin();
  }

  virtual ID update() {
    bool aPressed = btnA.isPressed();
    bool bPressed = btnB.isPressed();

    if ((aPressed && btnB.isHeld()) || (bPressed && btnA.isHeld())) return ID::BTN_BOTH;
    if (aPressed) return ID::BTN_A;
    if (bPressed) return ID::BTN_B;

    return ID::NONE;
  }
};
