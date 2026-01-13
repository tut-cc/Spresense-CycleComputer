#include "App.h"
#include "Config.h"
#include "ui/Input.h"

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

// Include "real" headers which now have virtual methods
#include "hardware/Button.h"
#include "hardware/Gnss.h"
#include "hardware/OLED.h"

// Define Mocks inheriting from the concrete classes
class MockOLED : public OLED {
public:
  // Using explicit override to ensure we matched the virtual signature
  MOCK_METHOD(void, clear, (), (override));
  MOCK_METHOD(void, display, (), (override));
  MOCK_METHOD(void, setTextSize, (int), (override));
  MOCK_METHOD(void, setTextColor, (int), (override));
  MOCK_METHOD(void, setCursor, (int, int), (override));
  MOCK_METHOD(void, print, (const char *), (override));
  MOCK_METHOD(void, drawLine, (int, int, int, int, int), (override));
  MOCK_METHOD(void, drawRect, (int, int, int, int, int), (override));
  MOCK_METHOD(void, fillRect, (int, int, int, int, int), (override));
  MOCK_METHOD(void, drawCircle, (int, int, int, int), (override));
  MOCK_METHOD(void, getTextBounds, (const char *, int16_t, int16_t, int16_t *, int16_t *, uint16_t *, uint16_t *), (override));
  MOCK_METHOD(int, getWidth, (), (const, override));
  MOCK_METHOD(int, getHeight, (), (const, override));
  MOCK_METHOD(void, begin, (), (override));

  MockOLED() : OLED() {
    // Set default behavior for dimensions to avoid crashes if called
    ON_CALL(*this, getWidth()).WillByDefault(Return(128));
    ON_CALL(*this, getHeight()).WillByDefault(Return(64));
  }
};

class MockGnssProvider : public Gnss {
public:
  mutable SpNavData data; // Helper to store test data

  MockGnssProvider() : Gnss() {
    memset(&data, 0, sizeof(data));
  }

  MOCK_METHOD(bool, begin, (), (override));
  MOCK_METHOD(void, update, (), (override));

  // We need to override getNavData to return our local 'data'
  // But strictly, hardware::Gnss::getNavData returns 'const SpNavData &' reference to its internal member.
  // We can return reference to our 'data' member.
  MOCK_METHOD(const SpNavData &, getNavData, (), (const, override));

  void setSpeed(float speedKmh) {
    data.velocity   = speedKmh / 3.6f;
    data.posFixMode = Fix3D;
  }
};

class MockButton : public Button {
public:
  MockButton(int pin) : Button(pin) {}
  MOCK_METHOD(void, begin, (), (override));
  MOCK_METHOD(bool, isPressed, (), (override));
  MOCK_METHOD(bool, isHeld, (), (const, override));
};

class AppTest : public ::testing::Test {
protected:
  NiceMock<MockOLED>         mockDisplay;
  NiceMock<MockGnssProvider> mockGnss;
  NiceMock<MockButton>      *mockBtnA;
  NiceMock<MockButton>      *mockBtnB;
  Input                     *input;
  App                       *app;

  void SetUp() override {
    mockBtnA = new NiceMock<MockButton>(Config::Pin::BTN_A);
    mockBtnB = new NiceMock<MockButton>(Config::Pin::BTN_B);

    ON_CALL(mockGnss, getNavData()).WillByDefault(ReturnRef(mockGnss.data));
    ON_CALL(*mockBtnA, isPressed()).WillByDefault(Return(false));
    ON_CALL(*mockBtnA, isHeld()).WillByDefault(Return(false));
    ON_CALL(*mockBtnB, isPressed()).WillByDefault(Return(false));
    ON_CALL(*mockBtnB, isHeld()).WillByDefault(Return(false));

    input = new Input(*mockBtnA, *mockBtnB);
    app   = new App(mockDisplay, mockGnss, *input);
  }

  void TearDown() override {
    delete app;
    delete input;
    delete mockBtnA;
    delete mockBtnB;
  }
};

TEST_F(AppTest, InitialModeIsSpeed) {
  EXPECT_CALL(mockDisplay, begin()).Times(1);
  EXPECT_CALL(mockGnss, begin()).Times(1);
  EXPECT_CALL(*mockBtnA, begin()).Times(1);
  EXPECT_CALL(*mockBtnB, begin()).Times(1);

  EXPECT_CALL(mockDisplay, display()).Times(AtLeast(1));

  app->begin();
  app->update();
}

TEST_F(AppTest, ModeChangeInput) {
  EXPECT_CALL(mockDisplay, display()).Times(AtLeast(2));

  app->begin();
  app->update();

  EXPECT_CALL(*mockBtnA, isPressed()).WillOnce(Return(true)).WillRepeatedly(Return(false));
  EXPECT_CALL(*mockBtnB, isPressed()).WillRepeatedly(Return(false));

  app->update();
}

TEST_F(AppTest, DisplayGPSSpeed) {
  app->begin();

  float testSpeed = 36.0;
  mockGnss.setSpeed(testSpeed);

  EXPECT_CALL(mockDisplay, print(_)).Times(AnyNumber());
  EXPECT_CALL(mockDisplay, print(testing::HasSubstr("36.0"))).Times(AtLeast(1));
  EXPECT_CALL(mockDisplay, display()).Times(AtLeast(1));

  app->update(); // First update initializes
  delay(100);    // Ensure time progresses
  app->update(); // Second update processes speed
}

TEST_F(AppTest, DisplayTime) {
  app->begin();

  EXPECT_CALL(mockDisplay, display()).Times(AnyNumber());
  EXPECT_CALL(mockDisplay, print(_)).Times(AnyNumber());

  // Transition 1
  EXPECT_CALL(*mockBtnA, isPressed()).WillOnce(Return(true)).WillRepeatedly(Return(false));
  app->update(); // Speed -> Trip

  // Transition 2
  EXPECT_CALL(*mockBtnA, isPressed()).WillOnce(Return(true)).WillRepeatedly(Return(false));
  app->update(); // Trip -> Time (Actually Max Speed)

  // Transition 3
  EXPECT_CALL(*mockBtnA, isPressed()).WillOnce(Return(true)).WillRepeatedly(Return(false));
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

  EXPECT_CALL(*mockBtnA, isPressed()).WillOnce(Return(true)).WillRepeatedly(Return(false));
  app->update(); // Change Mode

  mockGnss.setSpeed(0.0f);

  EXPECT_CALL(*mockBtnA, isPressed()).WillOnce(Return(true)).RetiresOnSaturation();
  EXPECT_CALL(*mockBtnB, isHeld()).WillOnce(Return(true)).RetiresOnSaturation();
  EXPECT_CALL(mockDisplay, print(testing::HasSubstr("0.0"))).Times(AtLeast(1));

  app->update();
}
