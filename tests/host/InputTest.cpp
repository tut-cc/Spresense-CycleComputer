#include "ui/Input.h"
#include "../mocks/Arduino.h"
#include "hardware/Button.h"

#include <gmock/gmock.h> // Required for NiceMock and ON_CALL
#include <gtest/gtest.h>

// Define MockButton class for testing
class MockButton : public Button {
public:
  MockButton(int pin) : Button(pin) {}
  MOCK_METHOD(void, begin, (), (override));
  MOCK_METHOD(bool, isPressed, (), (override));
  MOCK_METHOD(bool, isHeld, (), (const, override));
};

using ::testing::NiceMock;
using ::testing::Return;

// Test fixture for Input
class InputTest : public ::testing::Test {
protected:
  Input                *input;
  NiceMock<MockButton> *mockBtnA;
  NiceMock<MockButton> *mockBtnB;

  void SetUp() override {
    mockBtnA = new NiceMock<MockButton>(Config::Pin::BTN_A);
    mockBtnB = new NiceMock<MockButton>(Config::Pin::BTN_B);

    ON_CALL(*mockBtnA, isPressed()).WillByDefault(Return(false));
    ON_CALL(*mockBtnA, isHeld()).WillByDefault(Return(false));
    ON_CALL(*mockBtnB, isPressed()).WillByDefault(Return(false));
    ON_CALL(*mockBtnB, isHeld()).WillByDefault(Return(false));

    input = new Input(*mockBtnA, *mockBtnB);
    // input->begin(); // begin() might not be needed with mock buttons, or its behavior needs to be mocked.
    // Removing as per the spirit of the provided change.
  }

  void TearDown() override {
    delete input;
    delete mockBtnA;
    delete mockBtnB;
  }
};

// Helper to simulate Button press with debounce
// Helpers to simulate button behavior
void pressButton(Input *im, NiceMock<MockButton> *btn) {
  EXPECT_CALL(*btn, isPressed()).WillOnce(Return(true));
  EXPECT_CALL(*btn, isHeld()).WillRepeatedly(Return(false));
}

void releaseButton(Input *im, NiceMock<MockButton> *btn) {
  EXPECT_CALL(*btn, isPressed()).WillOnce(Return(false));
  EXPECT_CALL(*btn, isHeld()).WillRepeatedly(Return(false));
}

TEST_F(InputTest, InitialStateReturnsNone) {
  EXPECT_EQ(input->update(), Input::ID::NONE);
}

TEST_F(InputTest, ButtonAPress) {
  pressButton(input, mockBtnA);
  EXPECT_EQ(input->update(), Input::ID::BTN_A);

  releaseButton(input, mockBtnA);
  // Release usually returns NONE unless logic triggers on release.
  // Current logic triggers on PRESS (falling edge stabilization).
  // So release should return NONE.
  EXPECT_EQ(input->update(), Input::ID::NONE);
}

TEST_F(InputTest, ButtonBPress) {
  pressButton(input, mockBtnB);
  EXPECT_EQ(input->update(), Input::ID::BTN_B);

  releaseButton(input, mockBtnB);
  EXPECT_EQ(input->update(), Input::ID::NONE);
}

TEST_F(InputTest, SimultaneousPress) {
  EXPECT_CALL(*mockBtnA, isPressed()).WillOnce(Return(true));
  EXPECT_CALL(*mockBtnB, isHeld()).WillOnce(Return(true));
  EXPECT_EQ(input->update(), Input::ID::BTN_BOTH);

  releaseButton(input, mockBtnA);
  releaseButton(input, mockBtnB);
  EXPECT_EQ(input->update(), Input::ID::NONE);
}

TEST_F(InputTest, ButtonRelease) {
  pressButton(input, mockBtnA);
  EXPECT_EQ(input->update(), Input::ID::BTN_A);

  releaseButton(input, mockBtnA);
  EXPECT_EQ(input->update(), Input::ID::NONE);
}
