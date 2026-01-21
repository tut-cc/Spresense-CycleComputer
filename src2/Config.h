#pragma once

#include <Arduino.h>

namespace Config {

// ハードウェアピン設定
namespace Pins {
constexpr int BUTTON_SELECT = PIN_D09; // モード切替ボタン
constexpr int BUTTON_PAUSE  = PIN_D04; // 一時停止ボタン
constexpr int VOLTAGE_SENSE = PIN_A5;  // バッテリー電圧監視用ADC
constexpr int LOW_BATT_LED  = PIN_D00; // 低電圧警告LED
} // namespace Pins

// OLED ディスプレイ設定
namespace Display {
constexpr int WIDTH   = 128;  // OLED横幅 (ピクセル)
constexpr int HEIGHT  = 64;   // OLED縦幅 (ピクセル)
constexpr int ADDRESS = 0x3C; // I2Cアドレス (SSD1306標準)
} // namespace Display

// ボタン入力設定
namespace Button {
constexpr unsigned long DEBOUNCE_MS     = 20;   // チャタリング防止時間
constexpr unsigned long SINGLE_PRESS_MS = 30;   // シングルプレス判定時間
constexpr unsigned long LONG_PRESS_MS   = 3000; // 長押し判定時間
} // namespace Button

// GNSS設定
namespace Gnss {
constexpr unsigned long SIGNAL_TIMEOUT_MS    = 3000; // GNSS信号ロスト判定時間
constexpr float         MIN_MOVING_SPEED_KMH = 0.5f; // 移動判定の最低速度
constexpr float SPEED_SMOOTHING = 0.3f; // EMA平滑化係数 (0.0-1.0、小さいほど滑らか)
} // namespace Gnss

// データ保存設定
namespace Storage {
constexpr unsigned long SAVE_INTERVAL_MS      = 30000;      // 自動保存間隔 (30秒)
constexpr unsigned long EEPROM_ADDR           = 0;          // EEPROM保存先アドレス
constexpr float         MAX_VALID_DISTANCE_KM = 1000000.0f; // 距離データ有効範囲
} // namespace Storage

// 電圧監視設定
namespace Voltage {
constexpr float LOW_THRESHOLD     = 1.0f;    // 低電圧警告閾値 (V)
constexpr float REFERENCE_VOLTAGE = 3.3f;    // ADC基準電圧 (V)
constexpr float ADC_MAX_VALUE     = 1023.0f; // ADC最大値 (10bit)
} // namespace Voltage

// UI更新設定
namespace UI {
constexpr unsigned long UPDATE_INTERVAL_MS = 500; // UI更新間隔
constexpr unsigned long BLINK_INTERVAL_MS  = 500; // 点滅間隔
constexpr int           MODE_COUNT         = 3;   // 表示モード数
} // namespace UI

// 時刻設定
namespace Time {
constexpr int TIMEZONE_OFFSET_HOURS = 9;    // JST = UTC + 9
constexpr int MIN_VALID_YEAR        = 2026; // GPS時刻の有効判定年
} // namespace Time

} // namespace Config
