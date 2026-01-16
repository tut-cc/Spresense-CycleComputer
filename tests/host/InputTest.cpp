#include "ui/Input.h"
#include <gtest/gtest.h>

// Mock helpers declared in mocks/Arduino.h
extern void          setPinState(int pin, int state);
extern unsigned long _mock_millis;

class InputTest : public ::testing::Test {
protected:
  Input    *input;
  const int PIN_SELECT = 10;
  const int PIN_PAUSE  = 11;

  void SetUp() override {
    _mock_millis = 1000;
    setPinState(PIN_SELECT, HIGH);
    setPinState(PIN_PAUSE, HIGH);

    input = new Input(PIN_SELECT, PIN_PAUSE);
    input->begin();
  }

  void TearDown() override {
    delete input;
  }

  void tick(unsigned long ms) {
    _mock_millis += ms;
  }

  void press(int pin) {
    setPinState(pin, LOW);
    input->update(); // Detect transition
    tick(60);        // Wait debounce
    input->update(); // Confirm press
  }

  void release(int pin) {
    setPinState(pin, HIGH);
    input->update(); // Detect transition
    tick(60);        // Wait debounce
    input->update(); // Confirm release
  }
};

TEST_F(InputTest, SingleClickSelect) {
  // 1. Press Select
  setPinState(PIN_SELECT, LOW);
  input->update(); // Start debounce
  tick(60);
  input->update(); // Registered press. Pending set.

  // 2. Wait for simultaneous delay
  tick(60);
  Input::ID id = input->update();
  EXPECT_EQ(id, Input::ID::SELECT);
}

TEST_F(InputTest, ResetOnSimultaneousRelease) {
  // 1. Press Both logic: need to hit them close together
  setPinState(PIN_SELECT, LOW);
  setPinState(PIN_PAUSE, LOW);
  input->update(); // Start debounce
  tick(60);

  // update() here will register BOTH as pressed in same frame
  // processSimultaneousPress should detect this and return true
  // blocking handleNewPress.
  input->update();

  // Should NOT trigger RESET yet (holding)
  Input::ID id = input->update();
  EXPECT_EQ(id, Input::ID::NONE);

  // Hold for a bit
  tick(1000);
  id = input->update();
  EXPECT_EQ(id, Input::ID::NONE);

  // Release Both
  setPinState(PIN_SELECT, HIGH);
  setPinState(PIN_PAUSE, HIGH);
  input->update(); // Start debounce release
  tick(60);

  // Should now trigger RESET
  id = input->update(); // Confirm release -> processLongPress -> RESET
  EXPECT_EQ(id, Input::ID::RESET);
}

TEST_F(InputTest, ResetLongOnHold) {
  // Press Both
  setPinState(PIN_SELECT, LOW);
  setPinState(PIN_PAUSE, LOW);
  input->update();
  tick(60);
  input->update(); // Registered stable LOW

  // Hold until long press (>3000ms)
  // We already waited 60ms. Need more.
  tick(3100);

  // Should trigger RESET_LONG immediately
  Input::ID id = input->update();
  EXPECT_EQ(id, Input::ID::RESET_LONG);

  // Continue holding...
  tick(100);
  id = input->update();
  EXPECT_EQ(id, Input::ID::NONE);

  // Release Both
  setPinState(PIN_SELECT, HIGH);
  setPinState(PIN_PAUSE, HIGH);
  input->update();
  tick(60);

  // Should NOT trigger RESET
  id = input->update();
  EXPECT_EQ(id, Input::ID::NONE);
}

TEST_F(InputTest, InterleavedPressNotReset) {
  // Press Select
  setPinState(PIN_SELECT, LOW);
  input->update();
  tick(60);
  input->update(); // Select Pending

  // Then Press Pause (Simultaneous detection)
  setPinState(PIN_PAUSE, LOW);
  input->update();
  tick(60);
  input->update(); // Pause pressed.

  // processSimultaneousPress should catch this state where both are held?
  // Input.h: isSimultaneous checks held state too.

  // Now holding both... wait 1 sec
  tick(1000);
  EXPECT_EQ(input->update(), Input::ID::NONE);

  // Release
  setPinState(PIN_SELECT, HIGH);
  setPinState(PIN_PAUSE, HIGH);
  input->update();
  tick(60);

  EXPECT_EQ(input->update(), Input::ID::RESET);
}
