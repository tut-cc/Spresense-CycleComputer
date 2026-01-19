#pragma once

#include <Arduino.h>

#include "../hardware/VoltageSensor.h"

constexpr int   WARN_LED              = PIN_D00;
constexpr int   VOLTAGE_PIN           = PIN_A5;
constexpr float LOW_VOLTAGE_THRESHOLD = 1.0f;

class VoltageMonitor {
private:
  VoltageSensor voltageSensor;

public:
  VoltageMonitor() : voltageSensor(VOLTAGE_PIN) {}

  void begin() {
    voltageSensor.begin();
    pinMode(WARN_LED, OUTPUT);
  }

  float update() {
    const float currentVoltage = voltageSensor.readVoltage();
    if (currentVoltage <= LOW_VOLTAGE_THRESHOLD) digitalWrite(WARN_LED, HIGH);
    else digitalWrite(WARN_LED, LOW);
    return currentVoltage;
  }
};
