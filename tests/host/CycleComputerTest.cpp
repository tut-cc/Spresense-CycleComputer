#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Arduino.h"
#include "system/CycleComputer.h"
#include "interfaces/IDisplay.h"

using ::testing::_;
using ::testing::StrEq;

// Mock Display
class MockDisplay : public IDisplay {
public:
    MOCK_METHOD(void, begin, (), (override));
    MOCK_METHOD(void, clear, (), (override));
    MOCK_METHOD(void, show, (DisplayDataType type, const char* value), (override));
    MOCK_METHOD(void, update, (), (override));
};

class CycleComputerTest : public ::testing::Test {
protected:
    MockDisplay mockDisplay;
    CycleComputer* computer;

    void SetUp() override {
        _mock_millis = 1000;
        _mock_pin_states.clear();
        computer = new CycleComputer(&mockDisplay);
    }

    void TearDown() override {
        delete computer;
    }
};

TEST_F(CycleComputerTest, Initialization) {
    EXPECT_CALL(mockDisplay, begin()).Times(1);
    // CycleComputer::begin only calls display->begin() components
    
    computer->begin();
}

TEST_F(CycleComputerTest, UpdateTriggersDisplayUpdate) {
    computer->begin();

    // Expect display update
    // CycleComputer calls show(), but not clear() directly (drivers might, but IDisplay interface usage here doesn't show it)
    EXPECT_CALL(mockDisplay, show(_, _)).Times(testing::AtLeast(1));
    
    _mock_millis += 200; // +200ms
    computer->update();
}
