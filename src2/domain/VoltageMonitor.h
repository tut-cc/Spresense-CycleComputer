#pragma once

#include <Arduino.h>

constexpr int   WARN_LED              = PIN_D00;
constexpr int   VOLTAGE_PIN           = PIN_A5;
constexpr float LOW_VOLTAGE_THRESHOLD = 1.0f;
constexpr float REFERENCE_VOLTAGE     = 3.3f;
constexpr float ADC_MAX_VALUE         = 1023.0f;

class VoltageMonitor {
public:
  void begin() {
    pinMode(VOLTAGE_PIN, INPUT);
    pinMode(WARN_LED, OUTPUT);
  }

  float update() {
    int   rawValue       = analogRead(VOLTAGE_PIN);
    float currentVoltage = (rawValue / ADC_MAX_VALUE) * REFERENCE_VOLTAGE;
    digitalWrite(WARN_LED, (currentVoltage <= LOW_VOLTAGE_THRESHOLD) ? HIGH : LOW);
    return currentVoltage;
  }
};
