#pragma once
/**
 * @file VoltageMonitor.h
 * @brief バッテリー電圧監視クラス
 *
 * 電圧を測定し、低電圧時に警告LEDを点灯させます。
 */

#include "../hardware/VoltageSensor.h"
#include <Arduino.h>

constexpr int   WARN_LED              = PIN_D00; ///< 警告LED接続ピン
constexpr int   VOLTAGE_PIN           = PIN_A5;  ///< 電圧測定ピン
constexpr float LOW_VOLTAGE_THRESHOLD = 1.0f;    ///< 低電圧警告しきい値(V)

/**
 * @class VoltageMonitor
 * @brief バッテリー電圧の監視と警告
 */
class VoltageMonitor {
private:
  VoltageSensor voltageSensor; ///< 電圧センサー

public:
  VoltageMonitor() : voltageSensor(VOLTAGE_PIN) {}

  /** @brief 初期化 */
  void begin() {
    voltageSensor.begin();
    pinMode(WARN_LED, OUTPUT);
  }

  /**
   * @brief 電圧を測定して警告LED制御
   * @return 現在の電圧(V)
   */
  float update() {
    const float currentVoltage = voltageSensor.readVoltage();
    // 低電圧ならLED点灯、そうでなければ消灯
    if (currentVoltage <= LOW_VOLTAGE_THRESHOLD) digitalWrite(WARN_LED, HIGH);
    else digitalWrite(WARN_LED, LOW);
    return currentVoltage;
  }
};
