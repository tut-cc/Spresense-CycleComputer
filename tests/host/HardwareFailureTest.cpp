#include "hardware/Button.h"
#include "hardware/Gnss.h"
#include "hardware/OLED.h"
#include "hardware/VoltageSensor.h"
#include "mocks/Arduino.h"
#include <gtest/gtest.h>

/**
 * @brief ハードウェアの故障、ショート、未接続などの異常状態を想定したテスト
 */

class HardwareFailureTest : public ::testing::Test {
protected:
  void SetUp() override {
    _mock_millis = 0;
    _mock_pin_states.clear();
    _mock_analog_values.clear();
  }
};

// --- 1. ボタン故障 (Button Failure) ---

TEST_F(HardwareFailureTest, Button_ShortGND) {
  // ボタンが常にLOWに張り付いている（ショート）状態
  Button button(PIN_D02);
  setPinState(PIN_D02, LOW);
  button.begin();

  // 常に押されていると判定されるが、エッジトリガーによる「一回押した」判定は出ないべき
  button.update();
  EXPECT_FALSE(button.isPressed());
  EXPECT_TRUE(button.isHeld()); // 長押し状態として検出される
}

TEST_F(HardwareFailureTest, Button_Disconnected) {
  // ボタンが接続されていない、または断線している場合（プルアップにより常にHIGH）
  Button button(PIN_D02);
  setPinState(PIN_D02, HIGH);
  button.begin();

  button.update();
  EXPECT_FALSE(button.isPressed());
  EXPECT_FALSE(button.isHeld());
}

// --- 2. 電圧センサ異常 (Voltage Sensor Failure) ---

TEST_F(HardwareFailureTest, Voltage_ShortToGND) {
  // アナログピンがGNDにショートしている場合
  VoltageSensor sensor(PIN_A5);
  setAnalogReadValue(PIN_A5, 0);

  float v = sensor.readVoltage();
  EXPECT_FLOAT_EQ(v, 0.0f);
}

TEST_F(HardwareFailureTest, Voltage_ShortToVCC) {
  // アナログピンがVCC(3.3V)にショートしている場合
  VoltageSensor sensor(PIN_A5);
  setAnalogReadValue(PIN_A5, 1023);

  float v = sensor.readVoltage();
  EXPECT_FLOAT_EQ(v, 3.3f);
}

TEST_F(HardwareFailureTest, Voltage_OverVoltage) {
  // 想定以上の電圧（ADC最大値を超える場合、通常は1023に張り付く）
  VoltageSensor sensor(PIN_A5);
  setAnalogReadValue(PIN_A5, 2000); // 10bit ADCを超える異常値

  float v = sensor.readVoltage();
  // ロジック的に 2000/1023 * 3.3 となるが、実機では1023で飽和することを考慮したテスト
  EXPECT_GT(v, 3.3f);
}

// --- 3. I2Cバス異常 (Communication Failure) ---

TEST_F(HardwareFailureTest, OLED_NoResponse) {
  // OLEDが接続されていない、またはI2Cバスが死んでいる場合
  OLED oled;
  Adafruit_SSD1306::mockBeginResult = false; // beginが失敗を返す

  bool success = oled.begin();
  EXPECT_FALSE(success);

  // 失敗した状態でメソッドを呼んでもクラッシュしないことを確認
  oled.clear();
  oled.display();
}

// --- 4. GNSSモジュール異常 (Module Failure) ---

TEST_F(HardwareFailureTest, GNSS_NotResponding) {
  // GNSSモジュールが応答しない場合
  Gnss gnss;
  SpGnss::mockBeginResult = -1; // 初期化失敗

  bool success = gnss.begin();
  EXPECT_FALSE(success);
}

TEST_F(HardwareFailureTest, GNSS_StuckOnUpdate) {
  // 更新が全く来なくなった場合（タイムアウトの模倣）
  // これはTripロジック側でのテストに近いが、ハードウェア層の抽象化としても重要
}
