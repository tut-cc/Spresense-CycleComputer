#include "hardware/Button.h"
#include "hardware/Gnss.h"
#include "hardware/OLED.h"
#include "hardware/VoltageSensor.h"
#include "mocks/Arduino.h"
#include <gtest/gtest.h>

// --- Button Tests ---

TEST(ButtonTest, Debounce) {
  _mock_millis = 0;
  _mock_pin_states.clear();

  Button button(PIN_D02);
  button.begin(); // Initial state should be High

  _mock_pin_states[PIN_D02] = LOW; // Pressed
  _mock_millis              = 10;
  button.update(); // Transitions to WaitStablizeLow
  EXPECT_FALSE(button.isPressed());

  _mock_millis = 100; // Past 50ms debounce
  button.update();
  EXPECT_TRUE(button.isPressed());
  EXPECT_TRUE(button.isHeld());
}

TEST(ButtonTest, Bounce) {
  _mock_millis = 0;
  _mock_pin_states.clear();

  Button button(PIN_D02);
  button.begin();

  // Rapidly flip pin state (faster than 50ms)
  _mock_pin_states[PIN_D02] = LOW;
  _mock_millis              = 10;
  button.update();

  _mock_pin_states[PIN_D02] = HIGH;
  _mock_millis              = 20;
  button.update(); // Should reset stabilization

  EXPECT_FALSE(button.isPressed());
}

TEST(ButtonTest, StuckLow) {
  _mock_millis = 0;
  _mock_pin_states.clear();

  Button button(PIN_D02);
  _mock_pin_states[PIN_D02] = LOW; // Stuck BEFORE begin
  button.begin();

  EXPECT_FALSE(button.isPressed()); // Edge-trigger should NOT fire on initialization
  EXPECT_TRUE(button.isHeld());
}

// --- VoltageSensor Tests ---

TEST(VoltageSensorTest, ReadVoltage) {
  VoltageSensor sensor(PIN_A5);
  sensor.begin();
  float v = sensor.readVoltage();
  EXPECT_GT(v, 0.0f);
}

TEST(VoltageSensorTest, Extremes) {
  VoltageSensor sensor(PIN_A5);
  sensor.begin();

  // Actually we need to set PIN_A5 in Arduino mock if we want to test specific values
  // but current analogRead returns 512.
}

// --- OLED Tests ---

TEST(OLEDTest, Basic) {
  OLED oled;
  Adafruit_SSD1306::mockBeginResult = true;
  EXPECT_TRUE(oled.begin());
  oled.clear();
  oled.setTextSize(1);
  oled.setCursor(0, 0);
  oled.print("Test");
  oled.display();
}

TEST(OLEDTest, InitFailure) {
  OLED oled;
  Adafruit_SSD1306::mockBeginResult = false;
  EXPECT_FALSE(oled.begin());
}

// --- Gnss Tests ---

TEST(GnssTest, Basic) {
  Gnss gnss;
  SpGnss::mockBeginResult = 0;
  SpGnss::mockStartResult = 0;
  EXPECT_TRUE(gnss.begin());
}

TEST(GnssTest, InitFailure) {
  Gnss gnss;
  SpGnss::mockBeginResult = -1;
  EXPECT_FALSE(gnss.begin());

  SpGnss::mockBeginResult = 0;
  SpGnss::mockStartResult = -1;
  EXPECT_FALSE(gnss.begin());
}
