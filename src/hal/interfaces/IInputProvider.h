#pragma once

#include "../../system/InputEvent.h"

namespace hal {

class IInputProvider {
public:
  virtual ~IInputProvider()                = default;
  virtual void                    begin()  = 0;
  virtual application::InputEvent update() = 0;
};

} // namespace hal
