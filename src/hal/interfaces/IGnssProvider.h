#pragma once

#include <cstddef>

namespace hal {

class IGnssProvider {
public:
  virtual ~IGnssProvider()                                  = default;
  virtual bool  begin()                                     = 0;
  virtual void  update()                                    = 0;
  virtual float getSpeedKmh() const                         = 0;
  virtual void  getTimeJST(char *buffer, size_t size) const = 0;
};

} // namespace hal
