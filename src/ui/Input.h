#pragma once

#include "../Config.h"
#include "../hardware/Button.h"

class Input {
public:
  enum class ID {
    NONE,
    SELECT,
    PAUSE,
    RESET,
  };

private:
  Button btnSelect;
  Button btnPause;

  ID            pendingEvent = ID::NONE;
  unsigned long pendingTime  = 0;

public:
  Input() : btnSelect(Config::Pin::BTN_A), btnPause(Config::Pin::BTN_B) {}

  void begin() {
    btnSelect.begin();
    btnPause.begin();
  }

  ID update() {
    const bool          selectPressed = btnSelect.isPressed();
    const bool          pausePressed  = btnPause.isPressed();
    const unsigned long now           = millis();

    if ((selectPressed && (pausePressed || btnPause.isHeld())) ||
        (pausePressed && (selectPressed || btnSelect.isHeld()))) {
      pendingEvent = ID::NONE;
      return ID::RESET;
    }

    if (pendingEvent != ID::NONE) {
      const bool otherPressed  = pendingEvent == ID::SELECT && pausePressed;
      const bool otherPressed2 = pendingEvent == ID::PAUSE && selectPressed;
      if (otherPressed || otherPressed2) {
        pendingEvent = ID::NONE;
        return ID::RESET;
      }

      if (Config::Input::SIMULTANEOUS_DELAY_MS <= now - pendingTime) {
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
};
