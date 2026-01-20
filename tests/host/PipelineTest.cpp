#include "../../src2/domain/TripLogic.h"
#include "../../src2/ui/FrameLogic.h"
#include "mocks/Arduino.h"
#include "mocks/GNSS.h"
#include <gtest/gtest.h>

// --- Pipeline Tests ---

class PipelineTest : public ::testing::Test {
protected:
  void SetUp() override { _mock_millis = 0; }

  TripState createInitialState() {
    TripState state;
    state.resetAll();
    return state;
  }

  GnssData createGnssData(float velocityKmh, SpFixMode fixMode, bool updated = true) {
    GnssData data;
    data.navData.velocity   = velocityKmh / 3.6f;
    data.navData.posFixMode = fixMode;
    data.navData.latitude   = 35.6812;
    data.navData.longitude  = 139.7671;
    data.timestamp          = millis();
    data.status             = updated ? UpdateStatus::Updated : UpdateStatus::NoChange;
    return data;
  }

  // ヘルパー: Pause切替
  void togglePause(TripState &state) {
    state.status =
        state.isPaused() ? TripStateBase::Status::Stopped : TripStateBase::Status::Paused;
    state.forceUpdate();
  }
};

// ========================================
// TripState操作のテスト
// ========================================

TEST_F(PipelineTest, ResetTrip) {
  TripState state      = createInitialState();
  state.time.elapsed   = 5000;
  state.distance.trip  = 10.5f;
  state.distance.total = 100.0f;
  state.speed.max      = 50.0f;

  state.resetTrip();

  // トリップデータのみリセット
  EXPECT_EQ(state.time.elapsed, 0);
  EXPECT_FLOAT_EQ(state.distance.trip, 0.0f);
  EXPECT_EQ(state.status, TripStateBase::Status::Stopped);

  // 累積データは保持
  EXPECT_FLOAT_EQ(state.distance.total, 100.0f);
  EXPECT_FLOAT_EQ(state.speed.max, 50.0f);
  EXPECT_EQ(state.updateStatus, UpdateStatus::ForceUpdate);
}

TEST_F(PipelineTest, ResetMaxSpeed) {
  TripState state     = createInitialState();
  state.speed.max     = 50.0f;
  state.distance.trip = 10.5f;

  state.resetMaxSpeed();

  EXPECT_FLOAT_EQ(state.speed.max, 0.0f);
  EXPECT_FLOAT_EQ(state.distance.trip, 10.5f);
  EXPECT_EQ(state.updateStatus, UpdateStatus::ForceUpdate);
}

TEST_F(PipelineTest, ResetAll) {
  TripState state      = createInitialState();
  state.time.elapsed   = 5000;
  state.distance.trip  = 10.5f;
  state.distance.total = 100.0f;
  state.speed.max      = 50.0f;

  state.resetAll();

  EXPECT_EQ(state.time.elapsed, 0);
  EXPECT_FLOAT_EQ(state.distance.trip, 0.0f);
  EXPECT_FLOAT_EQ(state.distance.total, 0.0f);
  EXPECT_FLOAT_EQ(state.speed.max, 0.0f);
  EXPECT_EQ(state.status, TripStateBase::Status::Stopped);
  EXPECT_EQ(state.updateStatus, UpdateStatus::ForceUpdate);
}

TEST_F(PipelineTest, TogglePause) {
  TripState state = createInitialState();
  state.status    = TripStateBase::Status::Stopped;

  // Stopped -> Paused
  togglePause(state);
  EXPECT_EQ(state.status, TripStateBase::Status::Paused);
  EXPECT_EQ(state.updateStatus, UpdateStatus::ForceUpdate);

  // Paused -> Stopped
  togglePause(state);
  EXPECT_EQ(state.status, TripStateBase::Status::Stopped);
}

TEST_F(PipelineTest, BlinkLogic) {
  TripState state = createInitialState();
  state.status    = TripStateBase::Status::Paused;
  GnssData gnss   = createGnssData(0.0f, Fix3D);

  SpGnssTime t = {2024, 1, 1, 12, 0, 0, 0};

  // Time 0: blink ON (sub.value should be empty)
  _mock_millis        = 0;
  DisplayFrame frame0 = FrameLogic::buildFrame(state, gnss, t, Mode::SPD_TIM);
  EXPECT_STREQ(frame0.sub.value, "");

  // Time 500: blink OFF (sub.value should have content)
  _mock_millis        = 500;
  DisplayFrame frame1 = FrameLogic::buildFrame(state, gnss, t, Mode::SPD_TIM);
  EXPECT_STRNE(frame1.sub.value, "");

  // Time 1000: blink ON
  _mock_millis        = 1000;
  DisplayFrame frame2 = FrameLogic::buildFrame(state, gnss, t, Mode::SPD_TIM);
  EXPECT_STREQ(frame2.sub.value, "");
}

TEST_F(PipelineTest, BlinkLogic_NoBlinkInOtherModes) {
  TripState state = createInitialState();
  state.status    = TripStateBase::Status::Paused;
  GnssData gnss   = createGnssData(0.0f, Fix3D);

  _mock_millis = 0; // Blink phase ON
  SpGnssTime t = {2024, 1, 1, 12, 0, 0, 0};

  // SPD_TIM -> should blink
  DisplayFrame frameSPD = FrameLogic::buildFrame(state, gnss, t, Mode::SPD_TIM);
  EXPECT_STREQ(frameSPD.sub.value, "");

  // AVG_ODO -> should NOT blink
  DisplayFrame frameAVG = FrameLogic::buildFrame(state, gnss, t, Mode::AVG_ODO);
  EXPECT_STRNE(frameAVG.sub.value, "");

  // MAX_CLK -> should NOT blink
  DisplayFrame frameMAX = FrameLogic::buildFrame(state, gnss, t, Mode::MAX_CLK);
  EXPECT_STRNE(frameMAX.sub.value, "");
}

// ========================================
// 表示データ生成のテスト（DisplayFrame直接）
// ========================================

TEST_F(PipelineTest, BuildFrame_SpdTim) {
  TripState state     = createInitialState();
  state.speed.current = 25.5f;
  state.time.elapsed  = 3665000; // 1:01:05

  GnssData   gnss = createGnssData(25.5f, Fix3D);
  SpGnssTime t    = {2024, 1, 1, 12, 0, 0, 0};

  _mock_millis       = 500; // no blink
  DisplayFrame frame = FrameLogic::buildFrame(state, gnss, t, Mode::SPD_TIM);

  EXPECT_STREQ(frame.header.modeSpeed, "SPD");
  EXPECT_STREQ(frame.header.modeTime, "Time");
  EXPECT_STREQ(frame.main.unit, "km/h");
  EXPECT_STREQ(frame.header.fixStatus, "3D");
}

TEST_F(PipelineTest, BuildFrame_AvgOdo) {
  TripState state      = createInitialState();
  state.speed.avg      = 18.3f;
  state.distance.total = 123.45f;

  GnssData   gnss = createGnssData(20.0f, Fix3D);
  SpGnssTime t    = {2024, 1, 1, 12, 0, 0, 0};

  DisplayFrame frame = FrameLogic::buildFrame(state, gnss, t, Mode::AVG_ODO);

  EXPECT_STREQ(frame.header.modeSpeed, "AVG");
  EXPECT_STREQ(frame.header.modeTime, "Odo");
  EXPECT_STREQ(frame.main.unit, "km/h");
  EXPECT_STREQ(frame.sub.unit, "km");
}

TEST_F(PipelineTest, BuildFrame_MaxClk) {
  TripState state = createInitialState();
  state.speed.max = 45.2f;

  GnssData gnss            = createGnssData(20.0f, Fix3D);
  gnss.navData.time.year   = 2026;
  gnss.navData.time.hour   = 10;
  gnss.navData.time.minute = 30;

  DisplayFrame frame = FrameLogic::buildFrame(state, gnss, gnss.navData.time, Mode::MAX_CLK);

  EXPECT_STREQ(frame.header.modeSpeed, "MAX");
  EXPECT_STREQ(frame.header.modeTime, "Clock");
  EXPECT_STREQ(frame.sub.value, "19:30");
}

// ========================================
// Trip計算のヘルパー関数テスト
// ========================================

TEST_F(PipelineTest, CalculateRawKmh) {
  EXPECT_FLOAT_EQ(TripLogic::calculateRawKmh(10.0f / 3.6f), 10.0f);
}

TEST_F(PipelineTest, HasFix) {
  EXPECT_TRUE(TripLogic::hasFix(Fix2D));
  EXPECT_TRUE(TripLogic::hasFix(Fix3D));
  EXPECT_FALSE(TripLogic::hasFix(FixInvalid));
}

TEST_F(PipelineTest, IsMoving) {
  EXPECT_TRUE(TripLogic::isMoving(true, 10.0f));
  EXPECT_FALSE(TripLogic::isMoving(false, 10.0f));
  EXPECT_FALSE(TripLogic::isMoving(true, 0.0f));
}

TEST_F(PipelineTest, CalculateAverageSpeed) {
  EXPECT_FLOAT_EQ(TripLogic::calculateAverageSpeed(10.0f, 3600000), 10.0f);
  EXPECT_FLOAT_EQ(TripLogic::calculateAverageSpeed(10.0f, 0), 0.0f);
}
