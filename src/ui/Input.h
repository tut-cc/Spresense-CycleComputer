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

  ID                         pendingEvent          = ID::NONE;
  unsigned long              pendingTime           = 0;
  static const unsigned long SIMULTANEOUS_DELAY_MS = 50;

public:
  Input() : btnA(Config::Pin::BTN_A), btnB(Config::Pin::BTN_B) {}

  void begin() {
    btnA.begin();
    btnB.begin();
  }

  ID update() {
    const bool aPressed = btnA.isPressed();
    const bool bPressed = btnB.isPressed();

    if ((aPressed && btnB.isHeld()) || (bPressed && btnA.isHeld())) {
      pendingEvent = ID::NONE;
      return ID::BTN_BOTH;
    }

    if (aPressed && bPressed) {
      pendingEvent = ID::NONE;
      return ID::BTN_BOTH;
    }

    const unsigned long now = millis();

    if (pendingEvent != ID::NONE) {
      bool otherPressed = false;
      if (pendingEvent == ID::BTN_A && bPressed) otherPressed = true;
      if (pendingEvent == ID::BTN_B && aPressed) otherPressed = true;

      if (otherPressed) {
        pendingEvent = ID::NONE;
        return ID::BTN_BOTH;
      }

      // Check timeout
      if (now - pendingTime >= SIMULTANEOUS_DELAY_MS) {
        ID confirmed = pendingEvent;
        pendingEvent = ID::NONE;
        return confirmed;
      }

      return ID::NONE;
    }

    if (aPressed) {
      pendingEvent = ID::BTN_A;
      pendingTime  = now;
      return ID::NONE;
    }
    if (bPressed) {
      pendingEvent = ID::BTN_B;
      pendingTime  = now;
      return ID::NONE;
    }

    return ID::NONE;
  }
};
