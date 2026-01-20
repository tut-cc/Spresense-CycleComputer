#pragma once

#include "../hardware/Button.h"

constexpr unsigned long SINGLE_PRESS_MS = 30;
constexpr unsigned long LONG_PRESS_MS   = 3000;

class Input {
public:
  enum class Event { NONE, SELECT, PAUSE, RESET, RESET_LONG };

private:
  enum class State { Idle, MayBeSingle, MayBeDoubleShort, MustBeDoubleLong };

  Button selectButton;
  Button pauseButton;

  State state                = State::Idle;
  Event potentialSingleEvent = Event::NONE;

  unsigned long stateEnterTime = 0;

public:
  Input(int selectButtonPin, int pauseButtonPin)
      : selectButton(selectButtonPin), pauseButton(pauseButtonPin) {}

  void begin() {
    selectButton.begin();
    pauseButton.begin();
  }

  Event update() {
    selectButton.update();
    pauseButton.update();

    const bool          selectPressed = selectButton.pressed;
    const bool          selectHeld    = selectButton.held;
    const bool          pausePressed  = pauseButton.pressed;
    const bool          pauseHeld     = pauseButton.held;
    const unsigned long now           = millis();

    switch (state) {
    case State::Idle:
      if (selectPressed && pausePressed) {
        changeState(State::MayBeDoubleShort, now);
        return Event::NONE;
      }
      if (selectPressed) {
        potentialSingleEvent = Event::SELECT;
        changeState(State::MayBeSingle, now);
        return Event::NONE;
      }
      if (pausePressed) {
        potentialSingleEvent = Event::PAUSE;
        changeState(State::MayBeSingle, now);
        return Event::NONE;
      }
      break;

    case State::MayBeSingle:
      if ((potentialSingleEvent == Event::SELECT && pausePressed) ||
          (potentialSingleEvent == Event::PAUSE && selectPressed)) {
        changeState(State::MayBeDoubleShort, now);
        return Event::NONE;
      }
      if (now - stateEnterTime > SINGLE_PRESS_MS) {
        changeState(State::Idle, now);
        return potentialSingleEvent;
      }
      break;

    case State::MayBeDoubleShort:
      if (!selectHeld || !pauseHeld) {
        changeState(State::Idle, now);
        return Event::RESET;
      }
      if (now - stateEnterTime > LONG_PRESS_MS) {
        changeState(State::MustBeDoubleLong, now);
        return Event::RESET_LONG;
      }
      break;

    case State::MustBeDoubleLong:
      if (!selectHeld && !pauseHeld) changeState(State::Idle, now);
      break;
    }

    return Event::NONE;
  }

private:
  void changeState(State newState, unsigned long now) {
    state          = newState;
    stateEnterTime = now;
  }
};
