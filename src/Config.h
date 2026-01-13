#pragma once

#include <Arduino.h>

namespace Config {

constexpr unsigned long DEBOUNCE_DELAY             = 50;
constexpr unsigned long DISPLAY_UPDATE_INTERVAL_MS = 100;

namespace Pin {

constexpr int BTN_A    = PIN_D09;
constexpr int BTN_B    = PIN_D04;
constexpr int WARN_LED = PIN_D00;

} // namespace Pin

namespace OLED {

constexpr int WIDTH   = 128;
constexpr int HEIGHT  = 64;
constexpr int ADDRESS = 0x3C;

} // namespace OLED

namespace Time {

constexpr int JST_OFFSET = 9;

}

} // namespace Config
