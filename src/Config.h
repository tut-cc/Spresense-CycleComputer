#pragma once

#include <Arduino.h>

namespace Config {

namespace Pin {

constexpr int BTN_A    = PIN_D00;
constexpr int BTN_B    = PIN_D01;
constexpr int WARN_LED = PIN_D02;

} // namespace Pin

constexpr unsigned long DEBOUNCE_DELAY             = 50;
constexpr unsigned long DISPLAY_UPDATE_INTERVAL_MS = 100;

namespace OLED {

constexpr int WIDTH   = 128;
constexpr int HEIGHT  = 64;
constexpr int ADDRESS = 0x3C;

} // namespace OLED

namespace Time {

constexpr int JST_OFFSET = 9;

}

namespace Power {

constexpr int BATTERY_LOW_THRESHOLD     = 3600; // 3.6V
constexpr int BATTERY_CHECK_INTERVAL_MS = 10000;
constexpr int LED_BLINK_INTERVAL_MS     = 500;

} // namespace Power

} // namespace Config
