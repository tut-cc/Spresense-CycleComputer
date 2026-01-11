#include "ui/Input.h"
#include "../mocks/Arduino.h"
#include <gtest/gtest.h>

// Test fixture for Input
class InputTest : public ::testing::Test {
protected:
  ui::Input *input;

  void SetUp() override {
    // Reset mocks
    // drivers::Button::resetMock(); // Removed
    input = new ui::Input();
    input->begin();

    // Ensure initial state is released (HIGH)
    // Ensure initial state is released (HIGH)
    setPinState(Config::Pin::BTN_A, HIGH);
    setPinState(Config::Pin::BTN_B, HIGH);
  }

  void TearDown() override {
    delete input;
  }
};

// Helper to simulate Button press with debounce
void pressButton(ui::Input *im, int pin) {
  setPinState(pin, LOW);
  im->update();       // Detect change (start debounce)
  _mock_millis += 70; // Wait > 50ms (Config::DEBOUNCE_DELAY assumed 50)
}

void releaseButton(ui::Input *im, int pin) {
  setPinState(pin, HIGH);
  im->update(); // Detect change
  _mock_millis += 70;
}

TEST_F(InputTest, InitialStateReturnsNone) {
  EXPECT_EQ(input->update(), ui::InputEvent::NONE);
}

TEST_F(InputTest, ButtonAPress) {
  pressButton(input, Config::Pin::BTN_A);
  EXPECT_EQ(input->update(), ui::InputEvent::BTN_A);

  releaseButton(input, Config::Pin::BTN_A);
  // Release usually returns NONE unless logic triggers on release.
  // Current logic triggers on PRESS (falling edge stabilization).
  // So release should return NONE.
  EXPECT_EQ(input->update(), ui::InputEvent::NONE);
}

TEST_F(InputTest, ButtonBPress) {
  pressButton(input, Config::Pin::BTN_B);
  EXPECT_EQ(input->update(), ui::InputEvent::BTN_B);

  releaseButton(input, Config::Pin::BTN_B);
  EXPECT_EQ(input->update(), ui::InputEvent::NONE);
}

TEST_F(InputTest, SimultaneousPress) {
  setPinState(Config::Pin::BTN_A, LOW);
  setPinState(Config::Pin::BTN_B, LOW);
  input->update(); // Detect both change

  _mock_millis += 70;
  EXPECT_EQ(input->update(), ui::InputEvent::BTN_BOTH);

  releaseButton(input, Config::Pin::BTN_A);
  releaseButton(input, Config::Pin::BTN_B);
  EXPECT_EQ(input->update(), ui::InputEvent::NONE);
}

TEST_F(InputTest, ButtonRelease) {
  pressButton(input, Config::Pin::BTN_A);
  EXPECT_EQ(input->update(), ui::InputEvent::BTN_A);

  releaseButton(input, Config::Pin::BTN_A);
  EXPECT_EQ(input->update(), ui::InputEvent::NONE);
}
