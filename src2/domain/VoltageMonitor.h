#pragma once

#include "../Config.h"
#include <Arduino.h>

class VoltageMonitor {
public:
  void begin() { pinMode(Config::Pins::LOW_BATT_LED, OUTPUT); }

  float update() {
    int   rawValue = analogRead(Config::Pins::VOLTAGE_SENSE);
    float currentVoltage =
        (rawValue / Config::Voltage::ADC_MAX_VALUE) * Config::Voltage::REFERENCE_VOLTAGE;
    digitalWrite(Config::Pins::LOW_BATT_LED,
                 (currentVoltage <= Config::Voltage::LOW_THRESHOLD) ? HIGH : LOW);
    return currentVoltage;
  }
};
