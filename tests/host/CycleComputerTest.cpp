#include "system/CycleComputer.h"
#include "Config.h"
#include "hal/interfaces/IDisplay.h"
#include "hal/interfaces/IGnssProvider.h"
#include "hal/interfaces/IInputProvider.h"
#include "system/InputEvent.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::AtLeast;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::StrEq;

// Mock OLEDDriver
class MockOLEDDriver : public hal::IDisplay {
public:
  MOCK_METHOD(void, begin, (), (override));
  MOCK_METHOD(void, show, (application::DisplayDataType, const char *), (override));
};

// Mock GnssProvider
class MockGnssProvider : public hal::IGnssProvider {
public:
  MOCK_METHOD(bool, begin, (), (override));
  MOCK_METHOD(void, update, (), (override));
  MOCK_METHOD(float, getSpeedKmh, (), (const, override));
  MOCK_METHOD(void, getTimeJST, (char *buffer, size_t size), (const, override));
};

// Mock InputProvider
class MockInputProvider : public hal::IInputProvider {
public:
  MOCK_METHOD(void, begin, (), (override));
  MOCK_METHOD(application::InputEvent, update, (), (override));
};

class CycleComputerTest : public ::testing::Test {
protected:
  NiceMock<MockOLEDDriver>    mockDisplay;
  NiceMock<MockGnssProvider>  mockGnss;
  NiceMock<MockInputProvider> mockInput;
  application::CycleComputer *computer;

  void SetUp() override {
    // Default behaviors
    ON_CALL(mockGnss, getSpeedKmh()).WillByDefault(Return(0.0f));
    ON_CALL(mockInput, update()).WillByDefault(Return(application::InputEvent::NONE));

    computer = new application::CycleComputer(&mockDisplay, mockGnss, mockInput);
  }

  void TearDown() override {
    delete computer;
  }
};

TEST_F(CycleComputerTest, InitialModeIsSpeed) {
  EXPECT_CALL(mockDisplay, begin()).Times(1);
  EXPECT_CALL(mockGnss, begin()).Times(1);
  EXPECT_CALL(mockInput, begin()).Times(1);

  // Default mode is SPEED.
  EXPECT_CALL(mockDisplay, show(application::DisplayDataType::SPEED, _)).Times(AtLeast(1));

  computer->begin();
  computer->update();
}

TEST_F(CycleComputerTest, ModeChangeInput) {
  EXPECT_CALL(mockDisplay, show(application::DisplayDataType::SPEED, _)).Times(AtLeast(1));
  EXPECT_CALL(mockDisplay, show(application::DisplayDataType::MAX_SPEED, _)).Times(AtLeast(1));

  computer->begin();

  // First update: No input, shows Speed
  computer->update();

  // Simulate Button A press
  EXPECT_CALL(mockInput, update()).WillOnce(Return(application::InputEvent::BTN_A)).WillRepeatedly(Return(application::InputEvent::NONE));

  // Second update: Button A -> Switch to MAX_SPEED
  computer->update();
}

TEST_F(CycleComputerTest, DisplayGPSSpeed) {
  computer->begin();

  float testSpeed = 15.5;
  EXPECT_CALL(mockGnss, getSpeedKmh()).WillRepeatedly(Return(testSpeed));

  // Expect display to show formatted speed string
  EXPECT_CALL(mockDisplay, show(application::DisplayDataType::SPEED, testing::HasSubstr("15.5"))).Times(AtLeast(1));

  computer->update();
}

TEST_F(CycleComputerTest, DisplayTime) {
  computer->begin();

  // Allow other display calls (e.g. intermediate modes)
  EXPECT_CALL(mockDisplay, show(_, _)).Times(AnyNumber());

  // Switch to TIME mode: SPEED -> MAX -> AVG -> TIME
  // We need 3 BTN_A events.
  EXPECT_CALL(mockInput, update())
      .WillOnce(Return(application::InputEvent::BTN_A)) // -> MAX
      .WillOnce(Return(application::InputEvent::BTN_A)) // -> AVG
      .WillOnce(Return(application::InputEvent::BTN_A)) // -> TIME
      .WillRepeatedly(Return(application::InputEvent::NONE));

  // Mock Time JST
  EXPECT_CALL(mockGnss, getTimeJST(_, _)).WillRepeatedly(testing::Invoke([](char *buffer, size_t size) { strncpy(buffer, "12:34", size); }));

  // Expect eventually TIME mode with "12:34"
  EXPECT_CALL(mockDisplay, show(application::DisplayDataType::TIME, StrEq("12:34"))).Times(AtLeast(1));

  // Run updates to process inputs and display
  computer->update(); // MAX
  computer->update(); // AVG
  computer->update(); // TIME
  computer->update(); // Display
}

TEST_F(CycleComputerTest, ResetData) {
  computer->begin();

  // Allow any display calls initially
  EXPECT_CALL(mockDisplay, show(_, _)).Times(AnyNumber());

  // 1. Accumulate some distance
  // TripComputer.update uses speed and millis.
  float testSpeed = 30.0f;
  ON_CALL(mockGnss, getSpeedKmh()).WillByDefault(Return(testSpeed));

  computer->update(); // Start moving

  // We need to simulate time passing for TripComputer.
  // Since we cannot mock internal millis() call easily (it calls global millis()), avoiding strict time checks.
  // We mainly test that BTN_BOTH triggers reset logic.

  // Switch to MAX_SPEED mode (1 press)
  EXPECT_CALL(mockInput, update())
      .WillOnce(Return(application::InputEvent::BTN_A))    // Go to MAX_SPEED
      .WillOnce(Return(application::InputEvent::BTN_BOTH)) // Reset
      .WillRepeatedly(Return(application::InputEvent::NONE));

  // Verify Reset clears data (Max Speed becomes 0.0)
  // Note: We need to set speed to 0 so new max speed isn't immediately 30.0 again after update?
  // Actually, if we are still moving at 30.0, max speed will update to 30.0 immediately.
  // So we should set speed to 0.0 before reset check.

  // Sequence:
  // 1. Update (BTN_A) -> MAX_SPEED. Speed is 30. Max Speed is 30.
  // 2. Change Speed to 0.
  // 3. Update (BTN_BOTH) -> Reset. Max Speed becomes 0.

  computer->update(); // To MAX_SPEED.

  // Change speed to 0 for next updates
  ON_CALL(mockGnss, getSpeedKmh()).WillByDefault(Return(0.0f));

  // Expect 0.0 after reset
  EXPECT_CALL(mockDisplay, show(application::DisplayDataType::MAX_SPEED, testing::HasSubstr("0.0"))).Times(AtLeast(1));

  computer->update(); // Reset happens here
  computer->update(); // Show updated 0.0
}
