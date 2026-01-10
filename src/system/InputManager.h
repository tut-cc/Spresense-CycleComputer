#pragma once

#include "../drivers/ButtonDriver.h"

namespace application {

enum class InputEvent {
  NONE,
  BTN_A,
  BTN_B,
  BTN_BOTH,
};

class InputManager {
private:
  drivers::ButtonDriver btnA;
  drivers::ButtonDriver btnB;
  unsigned long lastInputTime = 0;

public:
  InputManager();
  void begin();
  InputEvent update();
};

} // namespace application
