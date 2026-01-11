#include "system/InputManager.h"
#include "../mocks/Arduino.h"
#include <gtest/gtest.h>

// Test fixture for InputManager
class InputManagerTest : public ::testing::Test {
protected:
  application::InputManager *inputManager;

  void SetUp() override {
    // Reset mocks
    // drivers::ButtonDriver::resetMock(); // Removed
    inputManager = new application::InputManager();
    inputManager->begin();

    // Ensure initial state is released (HIGH)
    // Ensure initial state is released (HIGH)
    setPinState(Config::Pin::BTN_A, HIGH);
    setPinState(Config::Pin::BTN_B, HIGH);
  }

  void TearDown() override {
    delete inputManager;
  }
};

// Helper to simulate ButtonDriver press with debounce
void pressButton(application::InputManager *im, int pin) {
  setPinState(pin, LOW);
  im->update();       // Detect change (start debounce)
  _mock_millis += 70; // Wait > 50ms (Config::DEBOUNCE_DELAY assumed 50)
}

void releaseButton(application::InputManager *im, int pin) {
  setPinState(pin, HIGH);
  im->update(); // Detect change
  _mock_millis += 70;
}

TEST_F(InputManagerTest, InitialStateReturnsNone) {
  EXPECT_EQ(inputManager->update(), application::InputEvent::NONE);
}

TEST_F(InputManagerTest, ButtonDriverAPress) {
  pressButton(inputManager, Config::Pin::BTN_A);
  EXPECT_EQ(inputManager->update(), application::InputEvent::BTN_A);

  releaseButton(inputManager, Config::Pin::BTN_A);
  // Release usually returns NONE unless logic triggers on release.
  // Current logic triggers on PRESS (falling edge stabilization).
  // So release should return NONE.
  EXPECT_EQ(inputManager->update(), application::InputEvent::NONE);
}

TEST_F(InputManagerTest, ButtonDriverBPress) {
  pressButton(inputManager, Config::Pin::BTN_B);
  EXPECT_EQ(inputManager->update(), application::InputEvent::BTN_B);

  releaseButton(inputManager, Config::Pin::BTN_B);
  EXPECT_EQ(inputManager->update(), application::InputEvent::NONE);
}

TEST_F(InputManagerTest, SimultaneousPress) {
  setPinState(Config::Pin::BTN_A, LOW);
  setPinState(Config::Pin::BTN_B, LOW);
  inputManager->update(); // Detect both change

  _mock_millis += 70;
  EXPECT_EQ(inputManager->update(), application::InputEvent::BTN_BOTH);

  releaseButton(inputManager, Config::Pin::BTN_A);
  releaseButton(inputManager, Config::Pin::BTN_B);
  EXPECT_EQ(inputManager->update(), application::InputEvent::NONE);
}

TEST_F(InputManagerTest, ButtonDriverRelease) {
  pressButton(inputManager, Config::Pin::BTN_A);
  EXPECT_EQ(inputManager->update(), application::InputEvent::BTN_A);

  releaseButton(inputManager, Config::Pin::BTN_A);
  EXPECT_EQ(inputManager->update(), application::InputEvent::NONE);
}
