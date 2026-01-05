/*
 * ファイル: Config.h
 * 説明: システム全体の設定パラメータ
 */

#pragma once

#include <Arduino.h>

#ifndef ARDUINO_ARCH_SPRESENSE
#define ARDUINO_ARCH_SPRESENSE
#endif

// デバッグ用定数
// #define DEBUGDAO

// ディスプレイタイプ定数
#define DISPLAY_SEVENSEG 1
#define DISPLAY_OLED 2

// ディスプレイタイプの選択（使用するものを1つだけ有効にしてください）
// #define DISPLAY_TYPE DISPLAY_SEVENSEG   // 7セグメントディスプレイ
#define DISPLAY_TYPE DISPLAY_OLED   // OLEDディスプレイ

// 省電力設定の条件付きコンパイル用フラグ
#ifdef ARDUINO_ARCH_SPRESENSE
#define ENABLE_POWER_SAVING 1
#endif

namespace Config {
    namespace Pin {
        // ボタン
        constexpr int BtnA = PIN_D00;
        constexpr int BtnB = PIN_D01;

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
        constexpr int D1_Colon = PIN_D03;
        constexpr int D2_Colon = PIN_D04;
        constexpr int D3_Colon = PIN_D05;
        constexpr int D4_Colon = PIN_D06;
        constexpr int Dot_Colon = PIN_D07;

        // OLED (I2C)
        constexpr int OLED_SCL = PIN_D15;
        constexpr int OLED_SDA = PIN_D14;

        // システム
        constexpr int WarnLed = PIN_D02;
    }

    namespace SevenSeg {
        // アノードコモンにおける数字表示用16進数
        // 0, 1, 2, ... 9, ハイフン
        constexpr unsigned char Numbers[] = {
            0xc0, 0xf9, 0xa4, 0xb0, 0x99, 
            0x92, 0x82, 0xf8, 0x80, 0x90, 
            0xbf
        };

        // ドット付表示用
        constexpr unsigned char NumbersWithDot[] = {
            0x40, 0x79, 0x24, 0x30, 0x19, 
            0x12, 0x02, 0x78, 0x00, 0x10, 
            0xbf
        };
    }

    namespace OLED {
        constexpr int Width = 128;
        constexpr int Height = 64;
        constexpr int Address = 0x3C;
    }

    namespace Power {
        #ifdef ARDUINO_ARCH_SPRESENSE
            constexpr bool IsSpresense = true;
            constexpr int BatteryLowThreshold = 3600;  // 3.6V
            constexpr int BatteryCheckIntervalMs = 10000;
            constexpr int LedBlinkIntervalMs = 500;
        #else
            constexpr bool IsSpresense = false;
            constexpr int BatteryLowThreshold = 0;
            constexpr int BatteryCheckIntervalMs = 10000;
            constexpr int LedBlinkIntervalMs = 500;
        #endif
    }
}
