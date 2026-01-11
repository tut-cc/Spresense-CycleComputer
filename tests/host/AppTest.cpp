#include "App.h"
#include "Config.h"
#include "ui/Input.h"
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

// ... (Mock Definitions are unchanged and skipped in this example if possible, but I must provide valid replacement for the whole or target parts)
// To keep it simple, I will replace the includes and the class definitions below line 70.
// I'll leave the Mocks alone by starting replacement at line 1. Wait, I can't mix-and-match easily without context.
// I will rewrite the whole file content to be safe and consistent with previous tool usage pattern.

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

class AppTest : public ::testing::Test {
protected:
  NiceMock<MockOLED>                                                                                 mockDisplay;
  NiceMock<MockGnssProvider>                                                                         mockGnss;
  NiceMock<MockButton>                                                                               mockBtnA;
  NiceMock<MockButton>                                                                               mockBtnB;
  ui::Input<NiceMock<MockButton>>                                                                   *input;
  application::App<NiceMock<MockOLED>, NiceMock<MockGnssProvider>, ui::Input<NiceMock<MockButton>>> *app;

  void SetUp() override {
    ON_CALL(mockGnss, getNavData()).WillByDefault(ReturnRef(mockGnss.data));
    // Default Button behavior: Not pressed, Not held
    ON_CALL(mockBtnA, isPressed()).WillByDefault(Return(false));
    ON_CALL(mockBtnA, isHeld()).WillByDefault(Return(false));
    ON_CALL(mockBtnB, isPressed()).WillByDefault(Return(false));
    ON_CALL(mockBtnB, isHeld()).WillByDefault(Return(false));

    input = new ui::Input<NiceMock<MockButton>>(mockBtnA, mockBtnB);
    app   = new application::App<NiceMock<MockOLED>, NiceMock<MockGnssProvider>, ui::Input<NiceMock<MockButton>>>(mockDisplay, mockGnss, *input);
  }

  void TearDown() override {
    delete app;
    delete input;
  }
};

TEST_F(AppTest, InitialModeIsSpeed) {
  EXPECT_CALL(mockDisplay, begin()).Times(1);
  EXPECT_CALL(mockGnss, begin()).Times(1);
  EXPECT_CALL(mockBtnA, begin()).Times(1);
  EXPECT_CALL(mockBtnB, begin()).Times(1);

  EXPECT_CALL(mockDisplay, display()).Times(AtLeast(1));

  app->begin();
  app->update();
}

TEST_F(AppTest, ModeChangeInput) {
  EXPECT_CALL(mockDisplay, display()).Times(AtLeast(2));

  app->begin();
  app->update();

  EXPECT_CALL(mockBtnA, isPressed()).WillOnce(Return(true)).WillRepeatedly(Return(false));
  EXPECT_CALL(mockBtnB, isPressed()).WillRepeatedly(Return(false));

  app->update();
}

TEST_F(AppTest, DisplayGPSSpeed) {
  app->begin();

  float testSpeed = 15.5;
  mockGnss.setSpeed(testSpeed);

  EXPECT_CALL(mockDisplay, print(_)).Times(AnyNumber());

  EXPECT_CALL(mockDisplay, print(testing::HasSubstr("15.5"))).Times(AtLeast(1));
  EXPECT_CALL(mockDisplay, display()).Times(AtLeast(1));

  app->update();
}

TEST_F(AppTest, DisplayTime) {
  app->begin();

  EXPECT_CALL(mockDisplay, display()).Times(AnyNumber());
  EXPECT_CALL(mockDisplay, print(_)).Times(AnyNumber());

  // Transition 1
  EXPECT_CALL(mockBtnA, isPressed()).WillOnce(Return(true)).WillRepeatedly(Return(false));
  app->update(); // Speed -> Trip

  // Transition 2
  EXPECT_CALL(mockBtnA, isPressed()).WillOnce(Return(true)).WillRepeatedly(Return(false));
  app->update(); // Trip -> Time (Actually Max Speed)

  // Transition 3
  EXPECT_CALL(mockBtnA, isPressed()).WillOnce(Return(true)).WillRepeatedly(Return(false));
  app->update(); // -> Time

  mockGnss.data.time.year   = 2025;
  mockGnss.data.time.hour   = 3; // UTC 3 -> JST 12
  mockGnss.data.time.minute = 34;

  EXPECT_CALL(mockDisplay, print(StrEq("12:34"))).Times(AtLeast(1));

  delay(1000);   // Advance time to trigger display update
  app->update(); // Render Time
}

TEST_F(AppTest, ResetData) {
  app->begin();

  EXPECT_CALL(mockDisplay, display()).Times(AnyNumber());
  EXPECT_CALL(mockDisplay, print(_)).Times(AnyNumber());

  float testSpeed = 30.0f;
  mockGnss.setSpeed(testSpeed);
  app->update();

  EXPECT_CALL(mockBtnA, isPressed()).WillOnce(Return(true)).WillRepeatedly(Return(false));
  app->update(); // Change Mode

  mockGnss.setSpeed(0.0f);

  EXPECT_CALL(mockBtnA, isPressed()).WillOnce(Return(true)).RetiresOnSaturation();
  EXPECT_CALL(mockBtnB, isHeld()).WillOnce(Return(true)).RetiresOnSaturation();

  EXPECT_CALL(mockDisplay, print(testing::HasSubstr("0.0"))).Times(AtLeast(1));

  app->update();
}
