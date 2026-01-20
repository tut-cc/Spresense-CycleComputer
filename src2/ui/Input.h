#pragma once

/**
 * @file Input.h
 * @brief ボタン入力の統合処理
 *
 * 複数ボタンの状態を監視し、シングルプレス、同時押し、
 * 長押しなどの入力イベントを判定します。
 */

#include "../common/Config.h"
#include "../hardware/Button.h"

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
      if (now - stateEnterTime > Config::Button::SINGLE_PRESS_MS) {
        changeState(State::Idle, now);
        return potentialSingleEvent;
      }
      break;

    case State::MayBeDoubleShort:
      if (!selectHeld || !pauseHeld) {
        changeState(State::Idle, now);
        return Event::RESET;
      }
      if (now - stateEnterTime > Config::Button::LONG_PRESS_MS) {
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
