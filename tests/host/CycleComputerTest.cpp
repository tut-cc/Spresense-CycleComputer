#include "system/CycleComputer.h"
#include "../mocks/Arduino.h"
#include "Config.h"
#include "drivers/OLEDDriver.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::AtLeast;
using ::testing::Return;
using ::testing::StrEq;

// Mock OLEDDriver
// Mock OLEDDriver
class MockOLEDDriver : public drivers::OLEDDriver {
public:
  MockOLEDDriver() : drivers::OLEDDriver(Wire) {}
  MOCK_METHOD(void, begin, (), (override));
  MOCK_METHOD(void, show, (application::DisplayDataType, const char *), (override));
};

class CycleComputerTest : public ::testing::Test {
protected:
  MockOLEDDriver mockDisplay;
  application::CycleComputer *computer;

  void SetUp() override {
    _mock_millis = 1000;
    _mock_pin_states.clear();
    computer = new application::CycleComputer(&mockDisplay);

    // Reset Pins to HIGH (released)
    setPinState(Config::Pin::BTN_A, HIGH);
    setPinState(Config::Pin::BTN_B, HIGH);
  }

  void TearDown() override {
    delete computer;
  }

  // Helper to simulate button press
  void pressButton(int pin) {
    setPinState(pin, LOW);
    computer->update();     // Detect change
    _mock_millis += 60;     // Wait debounce
    computer->update();     // Trigger event
    setPinState(pin, HIGH); // Release
    computer->update();
    _mock_millis += 60;
    computer->update();
  }

  void pressBothButtons() {
    setPinState(Config::Pin::BTN_A, LOW);
    setPinState(Config::Pin::BTN_B, LOW);
    computer->update();
    _mock_millis += 60;
    computer->update(); // Trigger BOTH
    setPinState(Config::Pin::BTN_A, HIGH);
    setPinState(Config::Pin::BTN_B, HIGH);
    computer->update();
    _mock_millis += 60;
    computer->update();
  }
};

TEST_F(CycleComputerTest, UpdateLoop) {
  EXPECT_CALL(mockDisplay, begin()).Times(1);
  // EXPECT_CALL(mockDisplay, clear()).Times(0); // clear() removed from interface
  // Actually CycleComputer::begin() calls display->begin().
  // CycleComputer::updateDisplay() calls display->show().

  // Init expectations
  EXPECT_CALL(mockDisplay, show(_, _)).Times(::testing::AtLeast(1));

  computer->begin();

  // Simulate some loop iterations
  computer->update();
}

TEST_F(CycleComputerTest, ModeChangeInput) {
  EXPECT_CALL(mockDisplay, begin()).Times(1);
  // Initial update
  EXPECT_CALL(mockDisplay, show(application::DisplayDataType::SPEED, _)).Times(AtLeast(1));
  // After button press, expect next mode (MAX_SPEED)
  EXPECT_CALL(mockDisplay, show(application::DisplayDataType::MAX_SPEED, _)).Times(AtLeast(1));

  computer->begin();

  // Simulate button press (SELECT)
  pressButton(Config::Pin::BTN_A);
}

TEST_F(CycleComputerTest, Initialization) {
  EXPECT_CALL(mockDisplay, begin()).Times(1);

  computer->begin();
}

TEST_F(CycleComputerTest, InitialModeIsSpeed) {
  EXPECT_CALL(mockDisplay, begin()).Times(1);
  // Default mode is SPEED.
  EXPECT_CALL(mockDisplay, show(application::DisplayDataType::SPEED, _)).Times(testing::AtLeast(1));

  computer->begin();
  computer->update();
}

TEST_F(CycleComputerTest, ModeSwitching) {
  EXPECT_CALL(mockDisplay, begin()).Times(1);
  computer->begin();

  // Initial: SPEED
  EXPECT_CALL(mockDisplay, show(application::DisplayDataType::SPEED, _)).Times(testing::AnyNumber());

  // Press A -> MAX_SPEED
  EXPECT_CALL(mockDisplay, show(application::DisplayDataType::MAX_SPEED, _)).Times(testing::AtLeast(1));
  pressButton(Config::Pin::BTN_A);

  // Press A -> AVG_SPEED
  EXPECT_CALL(mockDisplay, show(application::DisplayDataType::AVG_SPEED, _)).Times(testing::AtLeast(1));
  pressButton(Config::Pin::BTN_A);
}

TEST_F(CycleComputerTest, DisplayGPSSpeed) {
  EXPECT_CALL(mockDisplay, begin()).Times(1);
  computer->begin();

  // Consume initial display call
  EXPECT_CALL(mockDisplay, show(_, _)).Times(AnyNumber());

  float testSpeed = 15.5;
  drivers::GPSWrapper::setMockSpeed(testSpeed);

  // Expect display to show formatted speed string
  EXPECT_CALL(mockDisplay, show(application::DisplayDataType::SPEED, testing::HasSubstr("15.5"))).Times(testing::AtLeast(1));

  computer->update();
}

TEST_F(CycleComputerTest, DisplayTime) {
  EXPECT_CALL(mockDisplay, begin()).Times(1);
  computer->begin();

  // Switch to TIME mode first
  // SPEED -> MAX_SPEED -> AVG_SPEED -> TIME
  EXPECT_CALL(mockDisplay, show(_, _)).Times(AnyNumber());
  pressButton(Config::Pin::BTN_A); // MAX
  pressButton(Config::Pin::BTN_A); // AVG
  pressButton(Config::Pin::BTN_A); // TIME

  // Set UTC time to 3:34, which is 12:34 JST (UTC+9)
  drivers::GPSWrapper::setMockTime(3, 34, 56);

  EXPECT_CALL(mockDisplay, show(application::DisplayDataType::TIME, StrEq("12:34"))).Times(testing::AtLeast(1));

  _mock_millis += 1000;
  computer->update();
}

TEST_F(CycleComputerTest, ResetData) {
  EXPECT_CALL(mockDisplay, begin()).Times(1);
  computer->begin();

  EXPECT_CALL(mockDisplay, show(_, _)).Times(AnyNumber());

  // 1. Accumulate some distance
  drivers::GPSWrapper::setMockSpeed(30.0f);
  // Simulate 1 hour passing (needs many updates or large time jump?)
  // CycleComputer calls tripComputer.update(speed, millis())
  // We can jump time.

  computer->update();      // Start moving
  _mock_millis += 3600000; // 1 hour
  computer->update();      // Update distance (~30km)

  // Verify Max Speed is recorded
  // Switch to MAX_SPEED mode (1 press)
  EXPECT_CALL(mockDisplay, show(application::DisplayDataType::MAX_SPEED, testing::HasSubstr("30.0"))).Times(testing::AtLeast(1));
  pressButton(Config::Pin::BTN_A); // SPEED -> MAX_SPEED
  computer->update();

  // 2. Perform Reset
  EXPECT_CALL(mockDisplay, show(application::DisplayDataType::MAX_SPEED, testing::HasSubstr("0.0"))).Times(testing::AtLeast(1));
  pressBothButtons();

  // 3. Verify Max Speed is 0
  _mock_millis += 1000;
  computer->update();
}
