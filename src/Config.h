#pragma once

#include <Arduino.h>

// デバッグ用定数
#define DEBUGDAO

// ディスプレイタイプ定数
#define DISPLAY_SEVENSEG 1
#define DISPLAY_OLED 2

// ディスプレイタイプの選択（使用するものを1つだけ有効にしてください）
// #define DISPLAY_TYPE DISPLAY_SEVENSEG   // 7セグメントディスプレイ
#define DISPLAY_TYPE DISPLAY_OLED  // OLEDディスプレイ

// 省電力機能有効化フラグ
#define ENABLE_POWER_SAVING

namespace Config {
    namespace Pin {
        // ボタン
        constexpr int BTN_A = PIN_D00;
        constexpr int BTN_B = PIN_D01;

        // 7セグメントシフトレジスタ
        constexpr int SDI = PIN_D09;
        constexpr int RCLK = PIN_D08;
        constexpr int SRCLK = PIN_D07;

        // 7セグメント桁指定 (Common Anode/Cathode selection pins)
        constexpr int D1 = PIN_D10;
        constexpr int D2 = PIN_D11;
        constexpr int D3 = PIN_D12;
        constexpr int D4 = PIN_D13;

        // 7セグメントコロン/ドット制御
        constexpr int D1_COLON = PIN_D03;
        constexpr int D2_COLON = PIN_D04;
        constexpr int D3_COLON = PIN_D05;
        constexpr int D4_COLON = PIN_D06;
        constexpr int DOT_COLON = PIN_D07;

        // OLED (I2C)
        constexpr int OLED_SCL = PIN_D15;
        constexpr int OLED_SDA = PIN_D14;

        // システム
        constexpr int WARN_LED = PIN_D02;
    }

    constexpr unsigned long DISPLAY_UPDATE_INTERVAL_MS = 0;

    namespace SevenSeg {
        // アノードコモンにおける数字表示用16進数
        // 0, 1, 2, ... 9, ハイフン
        constexpr unsigned char NUMBERS[] = {0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92,
                                            0x82, 0xf8, 0x80, 0x90, 0xbf};

        // ドット付表示用
        constexpr unsigned char NUMBERS_WITH_DOT[] = {0x40, 0x79, 0x24, 0x30, 0x19, 0x12,
                                                    0x02, 0x78, 0x00, 0x10, 0xbf};
    }

    namespace OLED {
        constexpr int WIDTH = 128;
        constexpr int HEIGHT = 64;
        constexpr int ADDRESS = 0x3C;
    }

    namespace Power {
        constexpr int BATTERY_LOW_THRESHOLD = 3600;  // 3.6V
        constexpr int BATTERY_CHECK_INTERVAL_MS = 10000;
        constexpr int LED_BLINK_INTERVAL_MS = 500;
    }
}
