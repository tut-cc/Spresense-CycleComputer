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

    const bool          selectPressed = selectButton.isPressed();
    const bool          selectHeld    = selectButton.isHeld();
    const bool          pausePressed  = pauseButton.isPressed();
    const bool          pauseHeld     = pauseButton.isHeld();
    const unsigned long now           = millis();

    switch (state) {
    case State::Idle: // ボタンが2つとも押されていない状態
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

    case State::MayBeSingle: // たぶんボタン1つ押しの状態
      if ((potentialSingleEvent == Event::SELECT && pausePressed) ||
          (potentialSingleEvent == Event::PAUSE && selectPressed)) {
        changeState(State::MayBeDoubleShort, now);
        return Event::NONE;
      }

      if (now - stateEnterTime > SINGLE_PRESS_MS) {
        changeState(State::Idle, now);
        return potentialSingleEvent; // 1ボタン短押しならモードごとの操作
      }
      break;

    case State::MayBeDoubleShort: // たぶんボタン2つ押しの状態
      if (!selectHeld || !pauseHeld) {
        changeState(State::Idle, now);
        return Event::RESET; // 2ボタン短押しならリセット
      }

      if (now - stateEnterTime > LONG_PRESS_MS) {
        changeState(State::MustBeDoubleLong, now);
        return Event::RESET_LONG; // 2ボタン長押しなら全データリセット
      }
      break;

    case State::MustBeDoubleLong: // ボタン2つ押しの状態
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
