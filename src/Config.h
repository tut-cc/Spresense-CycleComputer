#pragma once

#include <Arduino.h>

#define DEBUG_MODE // デバッグ用定数

namespace Config {
    namespace Pin {
        // ボタン
        constexpr int BTN_A = PIN_D00;
        constexpr int BTN_B = PIN_D01;

        // OLED (I2C)
        constexpr int OLED_SCL = PIN_D15;
        constexpr int OLED_SDA = PIN_D14;

        // システム
        constexpr int WARN_LED = PIN_D02;
    }

    constexpr unsigned long DISPLAY_UPDATE_INTERVAL_MS = 100;

    namespace OLED {
        constexpr int WIDTH = 128;
        constexpr int HEIGHT = 64;
        constexpr int ADDRESS = 0x3C;
    }

    namespace Time {
        constexpr int TIMEZONE_OFFSET = 9;  // JST (+9)
    }

    namespace Trip {
        constexpr float MOVE_THRESHOLD_KMH = 3.0f;  // 「移動中」とみなすための閾値
    }

    namespace Power {
        constexpr int BATTERY_LOW_THRESHOLD = 3600;  // 3.6V
        constexpr int BATTERY_CHECK_INTERVAL_MS = 10000;
        constexpr int LED_BLINK_INTERVAL_MS = 500;
    }
}
