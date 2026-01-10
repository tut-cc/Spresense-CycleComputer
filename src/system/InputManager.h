#pragma once

#include "../drivers/Button.h"

enum class InputEvent {
  BTN_A,
  BTN_B,
  BTN_BOTH,
  NONE,
};

class InputManager {
private:
  Button btnA;
  Button btnB;

public:
  InputManager();
  void begin();
  InputEvent update();
};
