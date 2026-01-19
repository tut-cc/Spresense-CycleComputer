#include "../../src2/common/DataStructures.h"
#include "../../src2/ui/UI.h"
#include "mocks/Arduino.h"
#include "mocks/DisplayLogger.h"
#include <gtest/gtest.h>

class OLEDTruthTest : public ::testing::Test {
protected:
  UI ui;

  void SetUp() override {
    DisplayLogger::clear();
    _mock_millis = 1000;
  }

  bool hasText(const std::string &expected) {
    for (const auto &call : DisplayLogger::calls) {
      if (call.type == DrawCall::Type::Text && call.text.find(expected) != std::string::npos) {
        return true;
      }
    }
    return false;
  }
};

TEST_F(OLEDTruthTest, RenderSPD_TIM) {
  DisplayFrame frame;
  frame.header.fixStatus = "3D";
  frame.header.modeSpeed = "SPD";
  frame.header.modeTime  = "Time";
  strcpy(frame.main.value, "25.4");
  frame.main.unit = "km/h";
  strcpy(frame.sub.value, "1:01:01");
  frame.sub.unit = "";

  ui.draw(frame);

  // Verify Header
  EXPECT_TRUE(hasText("3D"));
  EXPECT_TRUE(hasText("SPD"));
  EXPECT_TRUE(hasText("Time"));

  // Verify Main Value (Speed)
  EXPECT_TRUE(hasText("25.4"));
  EXPECT_TRUE(hasText("km/h"));

  // Verify Sub Value (Time)
  // 3661s -> 1:01:01
  EXPECT_TRUE(hasText("1:01:01"));
}

TEST_F(OLEDTruthTest, RenderAVG_ODO) {
  DisplayFrame frame;
  frame.header.fixStatus = "2D";
  frame.header.modeSpeed = "AVG";
  frame.header.modeTime  = "Odo";
  strcpy(frame.main.value, "18.5");
  frame.main.unit = "km/h";
  strcpy(frame.sub.value, "123.45");
  frame.sub.unit = "km";

  ui.draw(frame);

  EXPECT_TRUE(hasText("2D"));
  EXPECT_TRUE(hasText("AVG"));
  EXPECT_TRUE(hasText("Odo"));
  EXPECT_TRUE(hasText("18.5"));
  EXPECT_TRUE(hasText("123.45"));
  EXPECT_TRUE(hasText("km"));
}

TEST_F(OLEDTruthTest, ResetMessage) {
  ui.showResetMessage();
  EXPECT_TRUE(hasText("RESETTING..."));
}

TEST_F(OLEDTruthTest, BlinkRendering) {
  // 1. Blink ON (should transmit empty string for sub value)
  DisplayFrame frameOn;
  frameOn.header.fixStatus = "3D";
  frameOn.header.modeSpeed = "SPD";
  frameOn.header.modeTime  = "Time";
  strcpy(frameOn.main.value, "0.0");
  frameOn.main.unit = "km/h";
  strcpy(frameOn.sub.value, "");
  frameOn.sub.unit = "";

  DisplayLogger::clear();
  ui.draw(frameOn);
  EXPECT_FALSE(hasText("12")); // Should NOT be visible

  // 2. Blink OFF (should transmit value)
  DisplayFrame frameOff;
  frameOff.header.fixStatus = "3D";
  frameOff.header.modeSpeed = "SPD";
  frameOff.header.modeTime  = "Time";
  strcpy(frameOff.main.value, "0.0");
  frameOff.main.unit = "km/h";
  strcpy(frameOff.sub.value, "00:12");
  frameOff.sub.unit = "";

  DisplayLogger::clear();
  ui.draw(frameOff);
  EXPECT_TRUE(hasText("00:12"));

  // 3. Blink ON again (should update frame and re-render)
  DisplayLogger::clear();
  ui.draw(frameOn);
  EXPECT_FALSE(hasText("00:12")); // Should disappear again
}

// ---------------------------------------------------------
// Pipeline logic needed for tests (using TripStateDataEx for methods)
// ---------------------------------------------------------
TEST_F(OLEDTruthTest, DummyToEnsureLink) {
  TripStateDataEx state;
  state.resetAll();
  EXPECT_EQ(state.status, TripStateData::Status::Stopped);
}
