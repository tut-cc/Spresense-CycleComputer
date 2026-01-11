#pragma once

namespace application {

class PowerManager {
private:
  unsigned long lastBatteryCheck = 0;
  bool          isLowBattery     = false;

public:
  PowerManager();

  void begin();
  void update();
};

} // namespace application
