#pragma once

#include "../Config.h"
#include <Arduino.h>

class BatteryMonitor {
public:
  void begin() { pinMode(Config::Pins::LOW_BATT_LED, OUTPUT); }

  float update() {
    const int   rawValue         = analogRead(Config::Pins::VOLTAGE_SENSE);
    const float voltageRatio     = rawValue / Config::Voltage::ADC_MAX_VALUE;
    const float voltage          = voltageRatio * Config::Voltage::REFERENCE_VOLTAGE;
    const bool  exceedsThreshold = Config::Voltage::REFERENCE_VOLTAGE < voltage;
    digitalWrite(Config::Pins::LOW_BATT_LED, exceedsThreshold ? LOW : HIGH);
    return voltage;
  }
};
