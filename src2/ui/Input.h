#pragma once

#include "../Config.h"

class Input {
public:
  enum class Event { NONE, SELECT, PAUSE, RESET, RESET_LONG };

private:
  struct Button {
    const int pin;
    bool      pressed = false, held = false;
    enum { High, WaitLow, Low, WaitHigh } state = High;
    unsigned long last                          = 0;
    Button(int p) : pin(p) {}
    inline void begin() {
      pinMode(pin, INPUT_PULLUP);
      state = digitalRead(pin) ? High : Low;
    }
    inline void update() {
      pressed           = false;
      bool          raw = digitalRead(pin);
      unsigned long now = millis();
      switch (state) {
      case High:
        if (!raw) {
          state = WaitLow;
          last  = now;
        }
        break;
      case WaitLow:
        if (raw) state = High;
        else if (now - last > Config::Button::DEBOUNCE_MS) {
          state   = Low;
          pressed = true;
        }
        break;
      case Low:
        if (raw) {
          state = WaitHigh;
          last  = now;
        }
        break;
      case WaitHigh:
        if (!raw) state = Low;
        else if (now - last > Config::Button::DEBOUNCE_MS) state = High;
        break;
      }
      held = (state == Low || state == WaitHigh);
    }
  };

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
        changeState(State::DblSrt, now);
        return Event::NONE;
      }
      if (b1.pressed) {
        pot = Event::SELECT;
        changeState(State::Single, now);
        return Event::NONE;
      }
      if (b2.pressed) {
        pot = Event::PAUSE;
        changeState(State::Single, now);
        return Event::NONE;
      }
      break;

    case State::Single:
      if ((pot == Event::SELECT && b2.pressed) || (pot == Event::PAUSE && b1.pressed)) {
        changeState(State::DblSrt, now);
        return Event::NONE;
      }
      if (now - last > Config::Button::SINGLE_PRESS_MS) {
        changeState(State::Idle, now);
        return pot;
      }
      break;

    case State::DblSrt:
      if (!b1.held || !b2.held) {
        changeState(State::Idle, now);
        return Event::RESET;
      }
      if (now - last > Config::Button::LONG_PRESS_MS) {
        changeState(State::DblLng, now);
        return Event::RESET_LONG;
      }
      break;

    case State::DblLng:
      if (!b1.held && !b2.held) changeState(State::Idle, now);
      break;
    }

    return Event::NONE;
  }

private:
  void changeState(State s, unsigned long n) {
    st   = s;
    last = n;
  }
};
