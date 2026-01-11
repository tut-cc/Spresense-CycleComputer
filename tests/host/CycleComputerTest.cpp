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
class MockOLED : public ui::GraphicsContext {
public:
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

// Mock InputProvider
class MockInputProvider {
public:
  MOCK_METHOD(void, begin, ());
  MOCK_METHOD(ui::InputEvent, update, ());
};

class CycleComputerTest : public ::testing::Test {
protected:
  NiceMock<MockOLED>                                                                                       mockDisplay;
  NiceMock<MockGnssProvider>                                                                               mockGnss;
  NiceMock<MockInputProvider>                                                                              mockInput;
  application::CycleComputer<NiceMock<MockOLED>, NiceMock<MockGnssProvider>, NiceMock<MockInputProvider>> *computer;

  void SetUp() override {
    ON_CALL(mockGnss, getNavData()).WillByDefault(ReturnRef(mockGnss.data));
    ON_CALL(mockInput, update()).WillByDefault(Return(ui::InputEvent::NONE));

    computer = new application::CycleComputer<NiceMock<MockOLED>, NiceMock<MockGnssProvider>, NiceMock<MockInputProvider>>(mockDisplay, mockGnss, mockInput);
  }

  void TearDown() override {
    delete computer;
  }
};

TEST_F(CycleComputerTest, InitialModeIsSpeed) {
  EXPECT_CALL(mockDisplay, begin()).Times(1);
  EXPECT_CALL(mockGnss, begin()).Times(1);
  EXPECT_CALL(mockInput, begin()).Times(1);

  // Expect render calls which result in display()
  EXPECT_CALL(mockDisplay, display()).Times(AtLeast(1));

  computer->begin();
  computer->update();
}

TEST_F(CycleComputerTest, ModeChangeInput) {
  EXPECT_CALL(mockDisplay, display()).Times(AtLeast(2));

  computer->begin();
  computer->update();

  EXPECT_CALL(mockInput, update()).WillOnce(Return(ui::InputEvent::BTN_A)).WillRepeatedly(Return(ui::InputEvent::NONE));
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

  EXPECT_CALL(mockInput, update()).WillOnce(Return(ui::InputEvent::BTN_A)).WillOnce(Return(ui::InputEvent::BTN_A)).WillOnce(Return(ui::InputEvent::BTN_A)).WillRepeatedly(Return(ui::InputEvent::NONE));

  mockGnss.data.time.year   = 2025;
  mockGnss.data.time.hour   = 3; // UTC 3 -> JST 12
  mockGnss.data.time.minute = 34;

  EXPECT_CALL(mockDisplay, print(StrEq("12:34"))).Times(AtLeast(1));

  computer->update();
  computer->update();
  computer->update();
  computer->update();
}

TEST_F(CycleComputerTest, ResetData) {
  computer->begin();

  EXPECT_CALL(mockDisplay, display()).Times(AnyNumber());
  EXPECT_CALL(mockDisplay, print(_)).Times(AnyNumber());

  float testSpeed = 30.0f;
  mockGnss.setSpeed(testSpeed);
  computer->update();

  EXPECT_CALL(mockInput, update()).WillOnce(Return(ui::InputEvent::BTN_A)).WillOnce(Return(ui::InputEvent::BTN_BOTH)).WillRepeatedly(Return(ui::InputEvent::NONE));

  computer->update();

  mockGnss.setSpeed(0.0f);

  EXPECT_CALL(mockDisplay, print(testing::HasSubstr("0.0"))).Times(AtLeast(1));

  computer->update();
  computer->update();
}
