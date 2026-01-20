#pragma once

#include "../Config.h"
#include <Arduino.h>

class Button {
public:
  const int pin;
  bool      pressed = false, held = false;
  enum { H, WL, L, WH } state = H;
  unsigned long last          = 0;

  Button(int p) : pin(p) {}
  void begin() {
    pinMode(pin, INPUT_PULLUP);
    state = digitalRead(pin) ? H : L;
  }

  void update() {
    pressed           = false;
    bool          raw = digitalRead(pin);
    unsigned long now = millis();
    switch (state) {
    case H:
      if (!raw) {
        state = WL;
        last  = now;
      }
      break;
    case WL:
      if (raw) state = H;
      else if (now - last > Config::Button::DEBOUNCE_MS) {
        state   = L;
        pressed = true;
      }
      break;
    case L:
      if (raw) {
        state = WH;
        last  = now;
      }
      break;
    case WH:
      if (!raw) state = L;
      else if (now - last > Config::Button::DEBOUNCE_MS) state = H;
      break;
    }
    held = (state == L || state == WH);
  }
};
