#pragma once

#include "../drivers/ButtonDriver.h"
#include "../hal/interfaces/IInputProvider.h"
#include "InputEvent.h"

namespace application {

class InputManager : public hal::IInputProvider {
private:
  drivers::ButtonDriver btnA;
  drivers::ButtonDriver btnB;
  unsigned long         lastInputTime = 0;

public:
  InputManager();
  void       begin() override;
  InputEvent update() override;
};

} // namespace application
