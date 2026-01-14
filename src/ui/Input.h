#pragma once

#include "../hardware/Button.h"

class Input {
public:
  enum class ID {
    NONE,
    BTN_A,
    BTN_B,
    BTN_BOTH,
  };

private:
  Button btnA;
  Button btnB;

public:
  Input() : btnA(Config::Pin::BTN_A), btnB(Config::Pin::BTN_B) {}

  void begin() {
    btnA.begin();
    btnB.begin();
  }

  ID update() {
    const bool aPressed = btnA.isPressed();
    const bool bPressed = btnB.isPressed();

    if ((aPressed && btnB.isHeld()) || (bPressed && btnA.isHeld())) return ID::BTN_BOTH;
    if (aPressed) return ID::BTN_A;
    if (bPressed) return ID::BTN_B;

    return ID::NONE;
  }
};
