#include "../../src2/DataStructures.h"
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
  DisplayData data;
  data.fixMode             = Fix3D;
  data.modeSpeedLabel      = "SPD";
  data.modeTimeLabel       = "Time";
  data.mainValue           = 25.4f;
  data.mainUnit            = "km/h";
  data.subType             = DisplayData::SubType::Duration;
  data.subValue.durationMs = 3661000; // 01:01:01
  data.subUnit             = "";
  data.shouldBlink         = false;
  data.updateStatus        = UpdateStatus::Updated;

  ui.draw(data);

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
  DisplayData data;
  data.fixMode             = Fix2D;
  data.modeSpeedLabel      = "AVG";
  data.modeTimeLabel       = "Odo";
  data.mainValue           = 18.5f;
  data.mainUnit            = "km/h";
  data.subType             = DisplayData::SubType::Distance;
  data.subValue.distanceKm = 123.45f;
  data.subUnit             = "km";
  data.shouldBlink         = false;
  data.updateStatus        = UpdateStatus::Updated;

  ui.draw(data);

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
  DisplayData data;
  data.fixMode             = Fix3D;
  data.modeSpeedLabel      = "SPD";
  data.modeTimeLabel       = "Time";
  data.mainValue           = 0.0f;
  data.mainUnit            = "km/h";
  data.subType             = DisplayData::SubType::Duration;
  data.subValue.durationMs = 12345;
  data.subUnit             = "";
  data.updateStatus        = UpdateStatus::NoChange; // Logic check: should render even if NoChange

  // 1. Blink ON (should transmit empty string for sub value)
  data.shouldBlink = true;
  DisplayLogger::clear();
  ui.draw(data);
  EXPECT_FALSE(hasText("12")); // Should NOT be visible (12 is part of 12345)
  // We can't easily check for "empty string" being drawn with hasText,
  // but we can check that the number is NOT drawn.

  // 2. Blink OFF (should transmit value)
  data.shouldBlink = false;
  DisplayLogger::clear();
  ui.draw(data);
  EXPECT_TRUE(hasText("00:12")); // 12.345s -> 00:12

  // 3. Blink ON again (should update frame and re-render)
  data.shouldBlink = true;
  DisplayLogger::clear();
  ui.draw(data);
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
