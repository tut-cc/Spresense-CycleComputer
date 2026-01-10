#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Arduino.h"
#include "Config.h"
#include "drivers/OLEDDriver.h"
#include "system/CycleComputer.h"

using ::testing::_;
using ::testing::StrEq;

// Mock Display
class MockDisplay : public OLEDDriver {
   public:
    MOCK_METHOD(bool, begin, (), (override));
    MOCK_METHOD(void, clear, (), (override));
    MOCK_METHOD(void, show, (DisplayDataType type, const char* value), (override));
};

class CycleComputerTest : public ::testing::Test {
   protected:
    MockDisplay mockDisplay;
    CycleComputer* computer;

    void SetUp() override {
        _mock_millis = 1000;
        _mock_pin_states.clear();
        computer = new CycleComputer(&mockDisplay);

        // Reset GPS Mocks
        GPSWrapper::setMockSpeed(0.0f);
        GPSWrapper::setMockTime(0, 0, 0);

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
        computer->update();      // Detect change
        _mock_millis += 60;      // Wait debounce
        computer->update();      // Trigger event
        setPinState(pin, HIGH);  // Release
        computer->update();
        _mock_millis += 60;
        computer->update();
    }

    void pressBothButtons() {
        setPinState(Config::Pin::BTN_A, LOW);
        setPinState(Config::Pin::BTN_B, LOW);
        computer->update();
        _mock_millis += 60;
        computer->update();  // Trigger BOTH
        setPinState(Config::Pin::BTN_A, HIGH);
        setPinState(Config::Pin::BTN_B, HIGH);
        computer->update();
        _mock_millis += 60;
        computer->update();
    }
};

TEST_F(CycleComputerTest, Initialization) {
    EXPECT_CALL(mockDisplay, begin()).Times(1);
    computer->begin();
}

TEST_F(CycleComputerTest, InitialModeIsSpeed) {
    computer->begin();

    // Default mode is SPEED.
    // Expect show to be called with SPEED type.
    EXPECT_CALL(mockDisplay, show(DisplayDataType::SPEED, _)).Times(testing::AtLeast(1));

    computer->update();
}

TEST_F(CycleComputerTest, ModeSwitching) {
    computer->begin();

    // Initial: SPEED.
    // When we press button, the first update() might trigger a SPEED display update
    // before the button press is registered and mode switches.
    EXPECT_CALL(mockDisplay, show(DisplayDataType::SPEED, _)).Times(testing::AnyNumber());

    // Press A -> TIME
    EXPECT_CALL(mockDisplay, show(DisplayDataType::TIME, _)).Times(testing::AtLeast(1));
    pressButton(Config::Pin::BTN_A);

    // Press A -> MAX_SPEED
    EXPECT_CALL(mockDisplay, show(DisplayDataType::MAX_SPEED, _)).Times(testing::AtLeast(1));
    pressButton(Config::Pin::BTN_A);
}

TEST_F(CycleComputerTest, DisplayGPSSpeed) {
    computer->begin();

    float testSpeed = 15.5;
    GPSWrapper::setMockSpeed(testSpeed);

    // Expect display to show formatted speed string
    // formatFloat(15.5, 4, 1) -> "15.5" ? or " 15.5" depending on implementation.
    // Let's verify it contains "15.5"
    EXPECT_CALL(mockDisplay, show(DisplayDataType::SPEED, testing::HasSubstr("15.5"))).Times(testing::AtLeast(1));

    computer->update();
}

TEST_F(CycleComputerTest, DisplayTime) {
    computer->begin();

    // Switch to TIME mode first
    pressButton(Config::Pin::BTN_A);

    // Set UTC time to 3:34, which is 12:34 JST (UTC+9)
    GPSWrapper::setMockTime(3, 34, 56);

    EXPECT_CALL(mockDisplay, show(DisplayDataType::TIME, StrEq("12:34"))).Times(testing::AtLeast(1));

    _mock_millis += 1000;
    computer->update();
}

TEST_F(CycleComputerTest, ResetData) {
    computer->begin();

    // 1. Accumulate some distance
    GPSWrapper::setMockSpeed(30.0f);
    // Simulate 1 hour passing (needs many updates or large time jump?)
    // CycleComputer calls tripComputer.update(speed, millis())
    // We can jump time.

    computer->update();       // Start moving
    _mock_millis += 3600000;  // 1 hour
    computer->update();       // Update distance (~30km)

    // Verify Max Speed is recorded
    // Switch to MAX_SPEED mode
    pressButton(Config::Pin::BTN_A);  // SPEED -> TIME
    pressButton(Config::Pin::BTN_A);  // TIME -> MAX_SPEED

    EXPECT_CALL(mockDisplay, show(DisplayDataType::MAX_SPEED, testing::HasSubstr("30.0"))).Times(testing::AtLeast(1));
    computer->update();

    // 2. Perform Reset
    pressBothButtons();

    // 3. Verify Max Speed is 0
    EXPECT_CALL(mockDisplay, show(DisplayDataType::MAX_SPEED, testing::HasSubstr("0.0"))).Times(testing::AtLeast(1));
    _mock_millis += 1000;
    computer->update();
}
