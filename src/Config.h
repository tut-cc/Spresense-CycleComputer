#pragma once

#include <Arduino.h>

namespace Config {

constexpr unsigned long DEBOUNCE_DELAY             = 50;
constexpr unsigned long DISPLAY_UPDATE_INTERVAL_MS = 100;

namespace Time {

constexpr int JST_OFFSET       = 9;
constexpr int VALID_YEAR_START = 2025;

} // namespace Time

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

namespace Renderer {

constexpr int16_t HEADER_HEIGHT    = 12;
constexpr int16_t FOOTER_HEIGHT    = 12;
constexpr int16_t MAIN_TEXT_SIZE   = 4;
constexpr int16_t HEADER_TEXT_SIZE = 1;
constexpr int16_t FOOTER_TEXT_SIZE = 1;

} // namespace Renderer

constexpr float MIN_MOVING_SPEED_KMH = 0.001f;

namespace Odometer {

constexpr float MIN_ABS   = 1e-6f;
constexpr float MIN_DELTA = 0.002f;
constexpr float MAX_DELTA = 1.0f;

} // namespace Odometer

namespace Input {

constexpr unsigned long SIMULTANEOUS_DELAY_MS = 50;

} // namespace Input

} // namespace Config
