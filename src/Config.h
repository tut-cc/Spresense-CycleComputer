/*
 * ファイル: Config.h
 * 説明: システム全体の設定パラメータ。
 *       ピン、ディスプレイ設定、および機能フラグを定義します。
 */

#pragma once

#include <Arduino.h>

// ディスプレイタイプ選択は条件付きコンパイルに使用されるため、#defineのままにする
// ディスプレイ設定
#define DISPLAY_LCD 1
#define DISPLAY_SEVENSEG 2
#define DISPLAY_I2C_LCD 3

// ディスプレイタイプの選択（使用するものを1つだけ有効にしてください）
// #define DISPLAY_TYPE DISPLAY_LCD        // 標準的なパラレル接続LCD
// #define DISPLAY_TYPE DISPLAY_SEVENSEG   // 7セグメントディスプレイ
#define DISPLAY_TYPE DISPLAY_I2C_LCD  // I2C接続LCD (0x27)

enum ButtonConfig { BTN_A_PIN = 8, BTN_B_PIN = 9 };

// LCD設定
enum LCDConfig { LCD_RS = 2, LCD_E = 3, LCD_D4 = 4, LCD_D5 = 5, LCD_D6 = 6, LCD_D7 = 7 };

// I2C LCD設定
enum I2CLCDConfig { LCD_I2C_ADDR = 0x27, LCD_COLS = 16, LCD_ROWS = 2 };

// 7セグメント設定
// シフトレジスタ
#define SDI_PIN PIN_D09
#define RCLK_PIN PIN_D08
#define SRCLK_PIN PIN_D07

// 桁の指定ピン
#define D1_PIN PIN_D10
#define D2_PIN PIN_D11
#define D3_PIN PIN_D12
#define D4_PIN PIN_D13

// アノードコモンにおける数字表示用16進数
#define DIS_NUM_0 0xc0
#define DIS_NUM_1 0xf9
#define DIS_NUM_2 0xa4
#define DIS_NUM_3 0xb0
#define DIS_NUM_4 0x99
#define DIS_NUM_5 0x92
#define DIS_NUM_6 0x82
#define DIS_NUM_7 0xf8
#define DIS_NUM_8 0x80
#define DIS_NUM_9 0x90
#define DIS_NUM_HYPHEN 0xbf

// ドット付表示をさせるための、アノードコモンにおける数字表示用16進数
#define DOT_NUM_0 0x40
#define DOT_NUM_1 0x79
#define DOT_NUM_2 0x24
#define DOT_NUM_3 0x30
#define DOT_NUM_4 0x19
#define DOT_NUM_5 0x12
#define DOT_NUM_6 0x02
#define DOT_NUM_7 0x78
#define DOT_NUM_8 0x00
#define DOT_NUM_9 0x10
#define DOT_NUM_HYPHEN 0xbf

// 省電力設定
#ifdef ARDUINO_ARCH_SPRESENSE
#define IS_SPRESENSE 1
#define ENABLE_POWER_SAVING 1
const int BATTERY_LOW_THRESHOLD = 3600;  // 3.6V (リミットは 3.5V-3.6V 付近)
const int BATTERY_CHECK_INTERVAL_MS = 10000;
const int LED_BLINK_INTERVAL_MS = 500;
const int WARN_LED = LED0;
#else
// Arduino またはその他のプラットフォーム
#undef ENABLE_POWER_SAVING
#endif
