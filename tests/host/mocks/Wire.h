#pragma once

#include <cstdint>

class TwoWire {
public:
  void begin();
  void setClock(uint32_t freq) {}
};

extern TwoWire Wire;
