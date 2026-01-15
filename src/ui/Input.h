#pragma once

#include "../hardware/Button.h"

class Input {
public:
  enum class ID {
    NONE,
    SELECT,
    PAUSE,
    RESET,
  };

  static constexpr unsigned long SIMULTANEOUS_DELAY_MS = 50;

private:
  Button btnSelect;
  Button btnPause;

  ID            pendingEvent = ID::NONE;
  unsigned long pendingTime  = 0;

public:
  Input(int pinSelect, int pinPause) : btnSelect(pinSelect), btnPause(pinPause) {}

  void begin() {
    btnSelect.begin();
    btnPause.begin();
  }

  ID update() {
    btnSelect.update();
    btnPause.update();

    const bool          selectPressed = btnSelect.wasPressed();
    const bool          pausePressed  = btnPause.wasPressed();
    const unsigned long now           = millis();

    if (isSimultaneous(selectPressed, pausePressed)) {
      pendingEvent = ID::NONE;
      return ID::RESET;
    }

    if (pendingEvent != ID::NONE) {
      if (resolvePendingEvent(selectPressed, pausePressed)) {
        pendingEvent = ID::NONE;
        return ID::RESET;
      }

      if (SIMULTANEOUS_DELAY_MS <= now - pendingTime) {
        ID confirmed = pendingEvent;
        pendingEvent = ID::NONE;
        return confirmed;
      }

      return ID::NONE;
    }

    if (selectPressed) {
      pendingEvent = ID::SELECT;
      pendingTime  = now;
      return ID::NONE;
    }

    if (pausePressed) {
      pendingEvent = ID::PAUSE;
      pendingTime  = now;
      return ID::NONE;
    }

    return ID::NONE;
  }

private:
  bool isSimultaneous(bool selectPressed, bool pausePressed) const {
    return (selectPressed && (pausePressed || btnPause.isHeld())) ||
           (pausePressed && (selectPressed || btnSelect.isHeld()));
  }

  bool resolvePendingEvent(bool selectPressed, bool pausePressed) const {
    const bool otherPressed  = pendingEvent == ID::SELECT && pausePressed;
    const bool otherPressed2 = pendingEvent == ID::PAUSE && selectPressed;
    return otherPressed || otherPressed2;
  }
};
