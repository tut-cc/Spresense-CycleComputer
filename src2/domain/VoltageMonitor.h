#pragma once

/**
 * @file VoltageMonitor.h
 * @brief バッテリー電圧監視機能
 *
 * ADCを使用してバッテリー電圧を監視し、
 * 低電圧時にLEDで警告を出します。
 */

#include "../common/Config.h"
#include <Arduino.h>

class VoltageMonitor {
public:
  void begin() {
    pinMode(Config::Pins::VOLTAGE_SENSE, INPUT);
    pinMode(Config::Pins::LOW_BATT_LED, OUTPUT);
  }

  float update() {
    int   rawValue = analogRead(Config::Pins::VOLTAGE_SENSE);
    float currentVoltage =
        (rawValue / Config::Voltage::ADC_MAX_VALUE) * Config::Voltage::REFERENCE_VOLTAGE;
    digitalWrite(Config::Pins::LOW_BATT_LED,
                 (currentVoltage <= Config::Voltage::LOW_THRESHOLD) ? HIGH : LOW);
    return currentVoltage;
  }
};
