#pragma once

#include "../drivers/Button.h"

namespace application {

enum class InputEvent {
  NONE,
  BTN_A,
  BTN_B,
  BTN_BOTH,
};

class InputManager {
private:
  drivers::Button btnA;
  drivers::Button btnB;
  unsigned long lastInputTime = 0;

public:
  InputManager();
  void begin();
  InputEvent update();
};

} // namespace application
