#pragma once

#include "../../system/DisplayData.h"

namespace hal {

class IDisplay {
public:
  virtual ~IDisplay()                                                     = default;
  virtual void begin()                                                    = 0;
  virtual void show(application::DisplayDataType type, const char *value) = 0;
};

} // namespace hal
