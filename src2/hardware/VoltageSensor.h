#pragma once
/**
 * @file VoltageSensor.h
 * @brief アナログ電圧センサークラス
 *
 * ADC（アナログ-デジタル変換器）を使用して電圧を測定します。
 */

#include <Arduino.h>

constexpr float REFERENCE_VOLTAGE = 3.3f;    ///< 基準電圧(V)
constexpr float ADC_MAX_VALUE     = 1023.0f; ///< ADC最大値(10bit)

/**
 * @class VoltageSensor
 * @brief アナログ電圧測定
 */
class VoltageSensor {
private:
  const int pin; ///< ADCピン番号

public:
  explicit VoltageSensor(int p) : pin(p) {}

  /** @brief ピンを入力モードに設定 */
  void begin() { pinMode(pin, INPUT); }

  /**
   * @brief 電圧を測定
   * @return 電圧(V)
   */
  float readVoltage() const {
    int rawValue = analogRead(pin);
    return (rawValue / ADC_MAX_VALUE) * REFERENCE_VOLTAGE;
  }
};
