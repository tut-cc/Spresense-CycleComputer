#include "PowerManager.h"

#include <LowPower.h>

#include "../Config.h"

namespace application {

PowerManager::PowerManager() {}

void PowerManager::begin() {
  pinMode(Config::Pin::WARN_LED, OUTPUT);
  digitalWrite(Config::Pin::WARN_LED, LOW);
  LowPower.begin();
  LowPower.clockMode(CLOCK_MODE_8MHz);
}

void PowerManager::update() {
  unsigned long currentMillis = millis();

  // 10秒ごとに電圧をチェック
  if (currentMillis - lastBatteryCheck >= Config::Power::BATTERY_CHECK_INTERVAL_MS) {
    lastBatteryCheck = currentMillis;
    int voltage = LowPower.getVoltage();
    // 電圧が有効であり（USBでは0を返すことがある）、かつ閾値を下回っているかを確認
    if (voltage > 0 && voltage < Config::Power::BATTERY_LOW_THRESHOLD) {
      isLowBattery = true;
    } else {
      isLowBattery = false;
      digitalWrite(Config::Pin::WARN_LED, LOW); // 安全な場合はLEDがオフであることを保証する
    }
  }

  // バッテリー残量が少ない場合、LEDを点滅させる
  if (isLowBattery) {
    // ブロッキングなしの単純な点滅のために時間の剰余を使用
    if ((currentMillis / Config::Power::LED_BLINK_INTERVAL_MS) % 2 == 0)
      digitalWrite(Config::Pin::WARN_LED, HIGH);
    else
      digitalWrite(Config::Pin::WARN_LED, LOW);
  }
}

} // namespace application
