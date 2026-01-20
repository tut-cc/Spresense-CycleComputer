#pragma once

class LowPowerClass {
public:
  void begin() {}
  void deepSleep(unsigned long seconds) {}
};

extern LowPowerClass LowPower;
