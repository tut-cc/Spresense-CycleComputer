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

    if (processSimultaneousPress(selectPressed, pausePressed)) { return ID::NONE; }

    event = processPendingEvent(selectPressed, pausePressed, now);
    if (event != ID::NONE || pendingEvent != ID::NONE) return event;

    return handleNewPress(selectPressed, pausePressed, now);
  }

private:
  ID processLongPress(unsigned long now) {
    // If not holding both, check if we just released after a short hold
    if (!btnSelect.isHeld() || !btnPause.isHeld()) {
      if (simultaneousStartTime != 0) {
        // Released
        bool wasLong = simultaneousLongPressTriggered;

        // Reset state
        simultaneousStartTime          = 0;
        simultaneousLongPressTriggered = false;

        // If it wasn't a long press, it's a normal simultaneous press (RESET)
        if (!wasLong) { return ID::RESET; }
      }
      return ID::NONE;
    }

    // Both held
    if (simultaneousStartTime == 0) {
      simultaneousStartTime = now;
      return ID::NONE;
    }

    if (now - simultaneousStartTime > LONG_PRESS_MS && !simultaneousLongPressTriggered) {
      simultaneousLongPressTriggered = true;
      return ID::RESET_LONG;
    }

    return ID::NONE;
  }

  bool processSimultaneousPress(bool selectPressed, bool pausePressed) {
    // Just consume pending events if we interpret this as a simultaneous action
    if (isSimultaneous(selectPressed, pausePressed)) {
      pendingEvent = ID::NONE;
      // Do NOT return ID::RESET here. Wait for release in processLongPress.
      return true;
    }
    return false;
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
      return ID::NONE;
    }

    if (pausePressed) {
      pendingEvent = ID::PAUSE;
      pendingTime  = now;
      return ID::NONE;
    }

    return ID::NONE;
  }

  bool isSimultaneous(bool selectPressed, bool pausePressed) const {
    const bool selectWithPause = selectPressed && (pausePressed || btnPause.isHeld());
    const bool pauseWithSelect = pausePressed && (selectPressed || btnSelect.isHeld());
    return selectWithPause || pauseWithSelect;
  }

  bool resolvePendingEvent(bool selectPressed, bool pausePressed) const {
    switch (pendingEvent) {
    case ID::SELECT:
      return pausePressed;
    case ID::PAUSE:
      return selectPressed;
    default:
      return false;
    }
  }
};
