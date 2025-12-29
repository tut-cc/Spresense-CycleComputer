/*
 * ファイル: Config.h
 * 説明: システム全体の設定パラメータ。
 *       ピン、ディスプレイ設定、および機能フラグを定義します。
 */

#ifndef CONFIG_H
#define CONFIG_H

// ディスプレイ設定
#define DISPLAY_LCD 1
#define DISPLAY_SEVENSEG 2
#define DISPLAY_I2C_LCD 3

// ディスプレイタイプの選択（使用するものを1つだけ有効にしてください）
// #define DISPLAY_TYPE DISPLAY_LCD        // 標準的なパラレル接続LCD
// #define DISPLAY_TYPE DISPLAY_SEVENSEG   // 7セグメントディスプレイ
#define DISPLAY_TYPE DISPLAY_I2C_LCD    // I2C接続LCD (0x27)

#define BTN_A_PIN 8
#define BTN_B_PIN 9
 
 // LCD設定
 #define LCD_RS 2
 #define LCD_E  3
 #define LCD_D4 4
 #define LCD_D5 5
 #define LCD_D6 6
 #define LCD_D7 7

// I2C LCD設定
#define LCD_I2C_ADDR 0x27
#define LCD_COLS 16
#define LCD_ROWS 2

// 省電力設定
#ifdef ARDUINO_ARCH_SPRESENSE
#define IS_SPRESENSE 1
#define ENABLE_POWER_SAVING 1
#define BATTERY_LOW_THRESHOLD 3600 // 3.6V (リミットは 3.5V-3.6V 付近)
#define BATTERY_CHECK_INTERVAL_MS 10000
#define LED_BLINK_INTERVAL_MS 500
#define WARN_LED LED0
#else
// Arduino またはその他のプラットフォーム
#undef ENABLE_POWER_SAVING
#endif

#endif
