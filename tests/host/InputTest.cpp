#include "ui/Input.h"
#include "../mocks/Arduino.h"
#include "drivers/Button.h"
#include <gtest/gtest.h>

// Test fixture for Input
class InputTest : public ::testing::Test {
protected:
  ui::Input<drivers::Button> *input;
  drivers::Button            *btnA;
  drivers::Button            *btnB;

  void SetUp() override {
    // Reset mocks
    // drivers::Button::resetMock(); // Removed
    btnA  = new drivers::Button(Config::Pin::BTN_A);
    btnB  = new drivers::Button(Config::Pin::BTN_B);
    input = new ui::Input<drivers::Button>(*btnA, *btnB);
    input->begin();

    // Ensure initial state is released (HIGH)
    // Ensure initial state is released (HIGH)
    setPinState(Config::Pin::BTN_A, HIGH);
    setPinState(Config::Pin::BTN_B, HIGH);
  }

  void TearDown() override {
    delete input;
    delete btnA;
    delete btnB;
  }
};

// Helper to simulate Button press with debounce
void pressButton(ui::Input<drivers::Button> *im, int pin) {
  setPinState(pin, LOW);
  im->update();       // Detect change (start debounce)
  _mock_millis += 70; // Wait > 50ms (Config::DEBOUNCE_DELAY assumed 50)
}

void releaseButton(ui::Input<drivers::Button> *im, int pin) {
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
