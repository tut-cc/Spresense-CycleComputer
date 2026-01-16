#pragma once

#include "../hardware/Button.h"

constexpr unsigned long SINGLE_PRESS_MS = 50;
constexpr unsigned long LONG_PRESS_MS   = 3000;

class Input {
public:
  enum class Event { NONE, SELECT, PAUSE, RESET, RESET_LONG };

private:
  enum class State { Idle, MayBeSingle, MayBeDoubleShort, MustBeDoubleLong };

  Button selectButton;
  Button pauseButton;

  State state             = State::Idle;
  Event potentialSingleID = Event::NONE;

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

    const bool          selectPressed = selectButton.wasPressed();
    const bool          selectHeld    = selectButton.isHeld();
    const bool          pausePressed  = pauseButton.wasPressed();
    const bool          pauseHeld     = pauseButton.isHeld();
    const unsigned long now           = millis();

    switch (state) {
    case State::Idle:
      if (selectPressed && pausePressed) {
        changeState(State::MayBeDoubleShort, now);
        return Event::NONE;
      }
      if (selectPressed) {
        potentialSingleID = Event::SELECT;
        changeState(State::MayBeSingle, now);
        return Event::NONE;
      }
      if (pausePressed) {
        potentialSingleID = Event::PAUSE;
        changeState(State::MayBeSingle, now);
        return Event::NONE;
      }
      break;

    case State::MayBeSingle:
      if ((potentialSingleID == Event::SELECT && pausePressed) ||
          (potentialSingleID == Event::PAUSE && selectPressed)) {
        changeState(State::MayBeDoubleShort, now);
        return Event::NONE;
      }

      if (now - stateEnterTime > SINGLE_PRESS_MS) {
        changeState(State::Idle, now);
        return potentialSingleID; // 1ボタン短押しはモードごとの操作
      }
      break;

    case State::MayBeDoubleShort:
      if (!selectHeld || !pauseHeld) {
        changeState(State::Idle, now);
        return Event::RESET; // 2ボタン短押しはリセット
      }

      if (now - stateEnterTime > LONG_PRESS_MS) {
        changeState(State::MustBeDoubleLong, now);
        return Event::RESET_LONG; // 2ボタン長押しは全データリセット
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
