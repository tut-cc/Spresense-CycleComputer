#include "CycleComputer.h"
#include "Config.h"

#include "ui/InputEvent.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::AtLeast;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::StrEq;

// Mock OLED
class MockOLED {
public:
  MOCK_METHOD(void, clear, ());
  MOCK_METHOD(void, display, ());
  MOCK_METHOD(void, setTextSize, (int));
  MOCK_METHOD(void, setTextColor, (int));
  MOCK_METHOD(void, setCursor, (int, int));
  MOCK_METHOD(void, print, (const char *));
  MOCK_METHOD(void, drawLine, (int, int, int, int, int));
  MOCK_METHOD(void, drawRect, (int, int, int, int, int));
  MOCK_METHOD(void, fillRect, (int, int, int, int, int));
  MOCK_METHOD(void, drawCircle, (int, int, int, int));

  MOCK_METHOD(void, getTextBounds, (const char *, int16_t, int16_t, int16_t *, int16_t *, uint16_t *, uint16_t *));

  MOCK_METHOD(int, getWidth, (), (const));
  MOCK_METHOD(int, getHeight, (), (const));

  MockOLED() {
    ON_CALL(*this, getWidth()).WillByDefault(Return(Config::OLED::WIDTH));
    ON_CALL(*this, getHeight()).WillByDefault(Return(Config::OLED::HEIGHT));
  }

  // Helper legacy method if needed, but primarily we use the interface now.
  MOCK_METHOD(void, begin, ());
};

// Mock GnssProvider
class MockGnssProvider {
public:
  mutable SpNavData data;
  void              setSpeed(float speedKmh) {
    data.velocity   = speedKmh / 3.6f;
    data.posFixMode = Fix3D;
  }

  MockGnssProvider() {
    memset(&data, 0, sizeof(data));
  }

  MOCK_METHOD(bool, begin, ());
  MOCK_METHOD(void, update, ());
  MOCK_METHOD(const SpNavData &, getNavData, (), (const));
};

// Mock Button
class MockButton {
public:
  MOCK_METHOD(void, begin, ());
  MOCK_METHOD(bool, isPressed, ());
  MOCK_METHOD(bool, isHeld, (), (const));
};

class CycleComputerTest : public ::testing::Test {
protected:
  NiceMock<MockOLED>                                                                                mockDisplay;
  NiceMock<MockGnssProvider>                                                                        mockGnss;
  NiceMock<MockButton>                                                                              mockBtnA;
  NiceMock<MockButton>                                                                              mockBtnB;
  application::CycleComputer<NiceMock<MockOLED>, NiceMock<MockGnssProvider>, NiceMock<MockButton>> *computer;

  void SetUp() override {
    ON_CALL(mockGnss, getNavData()).WillByDefault(ReturnRef(mockGnss.data));
    // Default Button behavior: Not pressed, Not held
    ON_CALL(mockBtnA, isPressed()).WillByDefault(Return(false));
    ON_CALL(mockBtnA, isHeld()).WillByDefault(Return(false));
    ON_CALL(mockBtnB, isPressed()).WillByDefault(Return(false));
    ON_CALL(mockBtnB, isHeld()).WillByDefault(Return(false));

    computer = new application::CycleComputer<NiceMock<MockOLED>, NiceMock<MockGnssProvider>, NiceMock<MockButton>>(mockDisplay, mockGnss, mockBtnA,
                                                                                                                    mockBtnB);
  }

  void TearDown() override {
    delete computer;
  }
};

TEST_F(CycleComputerTest, InitialModeIsSpeed) {
  EXPECT_CALL(mockDisplay, begin()).Times(1);
  EXPECT_CALL(mockGnss, begin()).Times(1);
  EXPECT_CALL(mockBtnA, begin()).Times(1);
  EXPECT_CALL(mockBtnB, begin()).Times(1);

  // Expect render calls which result in display()
  EXPECT_CALL(mockDisplay, display()).Times(AtLeast(1));

  computer->begin();
  computer->update();
}

TEST_F(CycleComputerTest, ModeChangeInput) {
  EXPECT_CALL(mockDisplay, display()).Times(AtLeast(2));

  computer->begin();
  computer->update();

  // Simulate BTN_A Press
  // Input checks isPressed() for both
  EXPECT_CALL(mockBtnA, isPressed()).WillOnce(Return(true)).WillRepeatedly(Return(false));
  EXPECT_CALL(mockBtnB, isPressed()).WillRepeatedly(Return(false));

  computer->update();
}

TEST_F(CycleComputerTest, DisplayGPSSpeed) {
  computer->begin();

  float testSpeed = 15.5;
  mockGnss.setSpeed(testSpeed);

  // Allow other print calls
  EXPECT_CALL(mockDisplay, print(_)).Times(AnyNumber());

  EXPECT_CALL(mockDisplay, print(testing::HasSubstr("15.5"))).Times(AtLeast(1));
  EXPECT_CALL(mockDisplay, display()).Times(AtLeast(1));

  computer->update();
}

TEST_F(CycleComputerTest, DisplayTime) {
  computer->begin();

  EXPECT_CALL(mockDisplay, display()).Times(AnyNumber());
  EXPECT_CALL(mockDisplay, print(_)).Times(AnyNumber());

  EXPECT_CALL(mockDisplay, display()).Times(AnyNumber());
  EXPECT_CALL(mockDisplay, print(_)).Times(AnyNumber());

  // simulate 3 A presses to get to Time Mode (Speed -> Trip -> Time)
  // Each press needs a frame where isPressed returns true.
  // We need to be careful about strict expectations.

  // Transition 1
  EXPECT_CALL(mockBtnA, isPressed()).WillOnce(Return(true)).WillRepeatedly(Return(false));
  computer->update(); // Speed -> Trip

  // Transition 2
  EXPECT_CALL(mockBtnA, isPressed()).WillOnce(Return(true)).WillRepeatedly(Return(false));
  computer->update(); // Trip -> Time (Actually Max Speed)

  // Transition 3
  EXPECT_CALL(mockBtnA, isPressed()).WillOnce(Return(true)).WillRepeatedly(Return(false));
  computer->update(); // -> Time

  mockGnss.data.time.year   = 2025;
  mockGnss.data.time.hour   = 3; // UTC 3 -> JST 12
  mockGnss.data.time.minute = 34;

  EXPECT_CALL(mockDisplay, print(StrEq("12:34"))).Times(AtLeast(1));

  delay(1000);        // Advance time to trigger display update
  computer->update(); // Render Time
}

TEST_F(CycleComputerTest, ResetData) {
  computer->begin();

  EXPECT_CALL(mockDisplay, display()).Times(AnyNumber());
  EXPECT_CALL(mockDisplay, print(_)).Times(AnyNumber());

  float testSpeed = 30.0f;
  mockGnss.setSpeed(testSpeed);
  computer->update();

  EXPECT_CALL(mockBtnA, isPressed()).WillOnce(Return(true)).WillRepeatedly(Return(false));
  computer->update(); // Change Mode

  // Simulate BTN_BOTH: (A Pressed && B Held) OR (B Pressed && A Held)
  mockGnss.setSpeed(0.0f);

  // Simulate BTN_BOTH: (A Pressed && B Held) OR (B Pressed && A Held)
  EXPECT_CALL(mockBtnA, isPressed()).WillOnce(Return(true)).RetiresOnSaturation();
  EXPECT_CALL(mockBtnB, isHeld()).WillOnce(Return(true)).RetiresOnSaturation();

  // Expect the display to update with "0.0" after reset
  EXPECT_CALL(mockDisplay, print(testing::HasSubstr("0.0"))).Times(AtLeast(1));

  computer->update();
}
