#pragma once

#include "../hardware/Button.h"

class Input {
public:
  enum class ID {
    NONE,
    SELECT,
    PAUSE,
    RESET,
    RESET_LONG,
  };

  static constexpr unsigned long SIMULTANEOUS_DELAY_MS = 50;
  static constexpr unsigned long LONG_PRESS_MS         = 3000;

private:
  Button btnSelect;
  Button btnPause;

  ID            pendingEvent                   = ID::NONE;
  unsigned long pendingTime                    = 0;
  unsigned long simultaneousStartTime          = 0;
  bool          simultaneousLongPressTriggered = false;

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

    ID event = processLongPress(now);
    if (event != ID::NONE) return event;

    event = processSimultaneousPress(selectPressed, pausePressed);
    if (event != ID::NONE) return event;

    event = processPendingEvent(selectPressed, pausePressed, now);
    if (event != ID::NONE || pendingEvent != ID::NONE) return event;

    return handleNewPress(selectPressed, pausePressed, now);
  }

private:
  ID processLongPress(unsigned long now) {
    if (btnSelect.isHeld() && btnPause.isHeld()) {
      if (simultaneousStartTime == 0) {
        simultaneousStartTime = now;
      } else if ((now - simultaneousStartTime > LONG_PRESS_MS) && !simultaneousLongPressTriggered) {
        simultaneousLongPressTriggered = true;
        return ID::RESET_LONG;
      }
    } else {
      simultaneousStartTime          = 0;
      simultaneousLongPressTriggered = false;
    }
    return ID::NONE;
  }

  ID processSimultaneousPress(bool selectPressed, bool pausePressed) {
    if (isSimultaneous(selectPressed, pausePressed)) {
      pendingEvent = ID::NONE;
      return ID::RESET;
    }
    return ID::NONE;
  }

  ID processPendingEvent(bool selectPressed, bool pausePressed, unsigned long now) {
    if (pendingEvent == ID::NONE) return ID::NONE;

    if (resolvePendingEvent(selectPressed, pausePressed)) {
      pendingEvent = ID::NONE;
      return ID::RESET;
    }

    if (now - pendingTime >= SIMULTANEOUS_DELAY_MS) {
      ID confirmed = pendingEvent;
      pendingEvent = ID::NONE;
      return confirmed;
    }

    return ID::NONE;
  }

  ID handleNewPress(bool selectPressed, bool pausePressed, unsigned long now) {
    if (selectPressed) {
      pendingEvent = ID::SELECT;
      pendingTime  = now;
    } else if (pausePressed) {
      pendingEvent = ID::PAUSE;
      pendingTime  = now;
    }
    return ID::NONE;
  }

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
