#pragma once

#include <Arduino.h>

namespace Pin {

constexpr int BTN_A       = PIN_D09;
constexpr int BTN_B       = PIN_D04;
constexpr int WARN_LED    = PIN_D00;
constexpr int VOLTAGE_PIN = A5;

} // namespace Pin

namespace Battery {
constexpr float LOW_VOLTAGE_THRESHOLD = 4.5f;
} // namespace Battery
