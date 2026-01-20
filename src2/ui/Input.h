#pragma once

#include "../Config.h"
#include "../hardware/Button.h"

class Input {
public:
  enum class Event { NONE, SELECT, PAUSE, RESET, RESET_LONG };

private:
  enum class State { Idle, Single, DblSrt, DblLng };
  Button        b1, b2;
  State         st   = State::Idle;
  Event         pot  = Event::NONE;
  unsigned long last = 0;

public:
  Input(int p1, int p2) : b1(p1), b2(p2) {}

  void begin() {
    b1.begin();
    b2.begin();
  }

  Event update() {
    b1.update();
    b2.update();
    unsigned long now = millis();
    switch (st) {
    case State::Idle:
      if (b1.pressed && b2.pressed) {
        ch(State::DblSrt, now);
        return Event::NONE;
      }
      if (b1.pressed) {
        pot = Event::SELECT;
        ch(State::Single, now);
        return Event::NONE;
      }
      if (b2.pressed) {
        pot = Event::PAUSE;
        ch(State::Single, now);
        return Event::NONE;
      }
      break;
    case State::Single:
      if ((pot == Event::SELECT && b2.pressed) || (pot == Event::PAUSE && b1.pressed)) {
        ch(State::DblSrt, now);
        return Event::NONE;
      }
      if (now - last > Config::Button::SINGLE_PRESS_MS) {
        ch(State::Idle, now);
        return pot;
      }
      break;
    case State::DblSrt:
      if (!b1.held || !b2.held) {
        ch(State::Idle, now);
        return Event::RESET;
      }
      if (now - last > Config::Button::LONG_PRESS_MS) {
        ch(State::DblLng, now);
        return Event::RESET_LONG;
      }
      break;
    case State::DblLng:
      if (!b1.held && !b2.held) ch(State::Idle, now);
      break;
    }
    return Event::NONE;
  }

private:
  void ch(State s, unsigned long n) {
    st   = s;
    last = n;
  }
};
