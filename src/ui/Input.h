#pragma once

#include "../Config.h"

struct Button {
  const int pin;
  bool      pressed = false, held = false;
  enum { High, WaitLow, Low, WaitHigh } state = High;
  unsigned long lastChangeTime                = 0;

  Button(int pinNumber) : pin(pinNumber) {}

  inline void begin() {
    pinMode(pin, INPUT_PULLUP);
    state = digitalRead(pin) ? High : Low;
  }

  inline void update() {
    pressed                   = false;
    bool          rawState    = digitalRead(pin);
    unsigned long currentTime = millis();

    switch (state) {
    case High:
      if (!rawState) {
        state          = WaitLow;
        lastChangeTime = currentTime;
      }
      break;

    case WaitLow:
      if (rawState) state = High;
      else if (currentTime - lastChangeTime > Config::Button::DEBOUNCE_MS) {
        state   = Low;
        pressed = true;
      }
      break;

    case Low:
      if (rawState) {
        state          = WaitHigh;
        lastChangeTime = currentTime;
      }
      break;

    case WaitHigh:
      if (!rawState) state = Low;
      else if (currentTime - lastChangeTime > Config::Button::DEBOUNCE_MS) state = High;
      break;
    }

    held = (state == Low || state == WaitHigh);
  }
};

class Input {
public:
  enum class Event { NONE, SELECT, PAUSE, RESET, RESET_LONG };

private:
  enum class State { Idle, SinglePressed, DoubleStarted, DoubleLongPressed };
  Button        buttonSelect, buttonPause;
  State         currentState  = State::Idle;
  Event         pendingEvent  = Event::NONE;
  unsigned long lastEventTime = 0;

public:
  Input(int selectPin, int pausePin) : buttonSelect(selectPin), buttonPause(pausePin) {}

  void begin() {
    buttonSelect.begin();
    buttonPause.begin();
  }

  Event update() {
    buttonSelect.update();
    buttonPause.update();
    unsigned long currentTime = millis();

    switch (currentState) {
    case State::Idle:
      if (buttonSelect.pressed && buttonPause.pressed) {
        changeState(State::DoubleStarted, currentTime);
        return Event::NONE;
      }
      if (buttonSelect.pressed) {
        pendingEvent = Event::SELECT;
        changeState(State::SinglePressed, currentTime);
        return Event::NONE;
      }
      if (buttonPause.pressed) {
        pendingEvent = Event::PAUSE;
        changeState(State::SinglePressed, currentTime);
        return Event::NONE;
      }
      break;

    case State::SinglePressed:
      if ((pendingEvent == Event::SELECT && buttonPause.pressed) ||
          (pendingEvent == Event::PAUSE && buttonSelect.pressed)) {
        changeState(State::DoubleStarted, currentTime);
        return Event::NONE;
      }
      if (currentTime - lastEventTime > Config::Button::SINGLE_PRESS_MS) {
        changeState(State::Idle, currentTime);
        return pendingEvent;
      }
      break;

    case State::DoubleStarted:
      if (!buttonSelect.held || !buttonPause.held) {
        changeState(State::Idle, currentTime);
        return Event::RESET;
      }
      if (currentTime - lastEventTime > Config::Button::LONG_PRESS_MS) {
        changeState(State::DoubleLongPressed, currentTime);
        return Event::RESET_LONG;
      }
      break;

    case State::DoubleLongPressed:
      if (!buttonSelect.held && !buttonPause.held) changeState(State::Idle, currentTime);
      break;
    }

    return Event::NONE;
  }

private:
  void changeState(State newState, unsigned long eventTime) {
    currentState  = newState;
    lastEventTime = eventTime;
  }
};
