#pragma once

#include <Arduino.h>

class PowerManager {
   private:
    unsigned long lastBatteryCheck = 0;
    bool isLowBattery = false;

   public:
    PowerManager();

    void begin();
    void update();
};
