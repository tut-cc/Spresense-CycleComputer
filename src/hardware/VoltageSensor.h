#pragma once

#include <Arduino.h>

class VoltageSensor {
private:
  const int              pin;
  static constexpr float REFERENCE_VOLTAGE = 3.3f;
  static constexpr float ADC_MAX_VALUE     = 1023.0f;

public:
  explicit VoltageSensor(int p) : pin(p) {}

  void begin() {
    pinMode(pin, INPUT);
  }

  float readVoltage() const {
    int rawValue = analogRead(pin);
    return (rawValue / ADC_MAX_VALUE) * REFERENCE_VOLTAGE;
  }
};
