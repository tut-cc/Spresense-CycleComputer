#include "../../src2/domain/DisplayLogic.h"
#include "../../src2/domain/InputLogic.h"
#include "../../src2/domain/TripLogic.h"
#include "mocks/Arduino.h"
#include "mocks/GNSS.h"
#include <gtest/gtest.h>

// --- Pipeline Tests ---

class PipelineTest : public ::testing::Test {
protected:
  void SetUp() override { _mock_millis = 0; }

  // ヘルパー: 初期状態を作成
  TripState createInitialState() {
    TripState state;
    state.resetAll();
    return state;
  }

  // ヘルパー: GNSSデータを作成
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
};

// ========================================
// ユーザー入力処理のテスト
// ========================================

TEST_F(PipelineTest, ResetType_Determination) {
  // RESET_LONG -> AllWithStorage
  EXPECT_EQ(InputLogic::determineResetType(Input::Event::RESET_LONG, Mode::SPD_TIM),
            InputLogic::ResetType::AllWithStorage);

  // RESET + SPD_TIM -> Trip
  EXPECT_EQ(InputLogic::determineResetType(Input::Event::RESET, Mode::SPD_TIM),
            InputLogic::ResetType::Trip);

  // RESET + AVG_ODO -> All
  EXPECT_EQ(InputLogic::determineResetType(Input::Event::RESET, Mode::AVG_ODO),
            InputLogic::ResetType::All);

  // RESET + MAX_CLK -> MaxSpeed
  EXPECT_EQ(InputLogic::determineResetType(Input::Event::RESET, Mode::MAX_CLK),
            InputLogic::ResetType::MaxSpeed);

  // その他 -> None
  EXPECT_EQ(InputLogic::determineResetType(Input::Event::NONE, Mode::SPD_TIM),
            InputLogic::ResetType::None);
}

TEST_F(PipelineTest, ApplyReset_Trip) {
  TripState state      = createInitialState();
  state.time.elapsed   = 5000;
  state.distance.trip  = 10.5f;
  state.distance.total = 100.0f;
  state.speed.max      = 50.0f;

  InputLogic::applyReset(state, InputLogic::ResetType::Trip);

  // トリップデータのみリセット
  EXPECT_EQ(state.time.elapsed, 0);
  EXPECT_FLOAT_EQ(state.distance.trip, 0.0f);
  EXPECT_EQ(state.status, TripStateBase::Status::Stopped);

  // 累積データは保持
  EXPECT_FLOAT_EQ(state.distance.total, 100.0f);
  EXPECT_FLOAT_EQ(state.speed.max, 50.0f);

  EXPECT_EQ(state.updateStatus, UpdateStatus::ForceUpdate);
}

TEST_F(PipelineTest, ApplyReset_MaxSpeed) {
  TripState state     = createInitialState();
  state.speed.max     = 50.0f;
  state.distance.trip = 10.5f;

  InputLogic::applyReset(state, InputLogic::ResetType::MaxSpeed);

  // 最高速度のみリセット
  EXPECT_FLOAT_EQ(state.speed.max, 0.0f);

  // 他のデータは保持
  EXPECT_FLOAT_EQ(state.distance.trip, 10.5f);

  EXPECT_EQ(state.updateStatus, UpdateStatus::ForceUpdate);
}

TEST_F(PipelineTest, ApplyReset_All) {
  TripState state      = createInitialState();
  state.time.elapsed   = 5000;
  state.distance.trip  = 10.5f;
  state.distance.total = 100.0f;
  state.speed.max      = 50.0f;

  InputLogic::applyReset(state, InputLogic::ResetType::All);

  // 全データリセット
  EXPECT_EQ(state.time.elapsed, 0);
  EXPECT_FLOAT_EQ(state.distance.trip, 0.0f);
  EXPECT_FLOAT_EQ(state.distance.total, 0.0f);
  EXPECT_FLOAT_EQ(state.speed.max, 0.0f);
  EXPECT_EQ(state.status, TripStateBase::Status::Stopped);

  EXPECT_EQ(state.updateStatus, UpdateStatus::ForceUpdate);
}

TEST_F(PipelineTest, ApplyPause) {
  TripState state = createInitialState();
  state.status    = TripStateBase::Status::Stopped;

  // Stopped -> Paused
  InputLogic::applyPause(state);
  EXPECT_EQ(state.status, TripStateBase::Status::Paused);
  EXPECT_EQ(state.updateStatus, UpdateStatus::ForceUpdate);

  // Paused -> Stopped
  InputLogic::applyPause(state);
  EXPECT_EQ(state.status, TripStateBase::Status::Stopped);
}

TEST_F(PipelineTest, BlinkLogic) {
  TripState state = createInitialState();
  state.status    = TripStateBase::Status::Paused;
  GnssData gnss   = createGnssData(0.0f, Fix3D);

  // Time 0: blink ON (shouldBlink = true)
  _mock_millis       = 0;
  SpGnssTime   t     = {2024, 1, 1, 12, 0, 0, 0};
  DisplayState data0 = DisplayLogic::create(state, gnss, t, Mode::SPD_TIM);
  EXPECT_TRUE(data0.shouldBlink);

  // Time 500: blink OFF
  _mock_millis       = 500;
  DisplayState data1 = DisplayLogic::create(state, gnss, t, Mode::SPD_TIM);
  EXPECT_FALSE(data1.shouldBlink);

  // Time 1000: blink ON
  _mock_millis       = 1000;
  DisplayState data2 = DisplayLogic::create(state, gnss, t, Mode::SPD_TIM);
  EXPECT_TRUE(data2.shouldBlink);
}

TEST_F(PipelineTest, BlinkLogic_NoBlinkInOtherModes) {
  TripState state = createInitialState();
  state.status    = TripStateBase::Status::Paused;
  GnssData gnss   = createGnssData(0.0f, Fix3D);

  _mock_millis = 0; // Blink phase ON
  SpGnssTime t = {2024, 1, 1, 12, 0, 0, 0};

  // SPD_TIM -> should blink
  DisplayState dataSPD = DisplayLogic::create(state, gnss, t, Mode::SPD_TIM);
  EXPECT_TRUE(dataSPD.shouldBlink);

  // AVG_ODO -> should NOT blink
  DisplayState dataAVG = DisplayLogic::create(state, gnss, t, Mode::AVG_ODO);
  EXPECT_FALSE(dataAVG.shouldBlink);

  // MAX_CLK -> should NOT blink
  DisplayState dataMAX = DisplayLogic::create(state, gnss, t, Mode::MAX_CLK);
  EXPECT_FALSE(dataMAX.shouldBlink);
}

TEST_F(PipelineTest, SwitchMode) {
  // SELECT -> 次のモード
  EXPECT_EQ(InputLogic::switchMode(Mode::SPD_TIM, Input::Event::SELECT), Mode::AVG_ODO);
  EXPECT_EQ(InputLogic::switchMode(Mode::AVG_ODO, Input::Event::SELECT), Mode::MAX_CLK);
  EXPECT_EQ(InputLogic::switchMode(Mode::MAX_CLK, Input::Event::SELECT), Mode::SPD_TIM);

  // その他 -> 変更なし
  EXPECT_EQ(InputLogic::switchMode(Mode::SPD_TIM, Input::Event::NONE), Mode::SPD_TIM);
}

TEST_F(PipelineTest, HandleUserInput_Pause) {
  TripState state = createInitialState();

  auto result = InputLogic::handleEvent(state, Mode::SPD_TIM, Input::Event::PAUSE);

  EXPECT_EQ(state.status, TripStateBase::Status::Paused);
  EXPECT_EQ(result.newMode, Mode::SPD_TIM);
  EXPECT_FALSE(result.shouldClearStorage);
}

TEST_F(PipelineTest, HandleUserInput_ResetLong) {
  TripState state      = createInitialState();
  state.distance.total = 100.0f;

  auto result = InputLogic::handleEvent(state, Mode::SPD_TIM, Input::Event::RESET_LONG);

  EXPECT_FLOAT_EQ(state.distance.total, 0.0f);
  EXPECT_TRUE(result.shouldClearStorage);
}

// ========================================
// 表示データ生成のテスト
// ========================================

TEST_F(PipelineTest, CreateDisplayState_SpdTim) {
  TripState state     = createInitialState();
  state.speed.current = 25.5f;
  state.time.elapsed  = 3665000; // 1:01:05

  GnssData   gnss = createGnssData(25.5f, Fix3D);
  SpGnssTime t    = {2024, 1, 1, 12, 0, 0, 0};

  DisplayState data = DisplayLogic::create(state, gnss, t, Mode::SPD_TIM);

  EXPECT_STREQ(data.modeSpeedLabel, "SPD");
  EXPECT_STREQ(data.modeTimeLabel, "Time");
  EXPECT_FLOAT_EQ(data.mainValue, 25.5f);
  EXPECT_STREQ(data.mainUnit, "km/h");
  EXPECT_EQ(data.subType, DisplayState::SubType::Duration);
  EXPECT_EQ(data.subValue.durationMs, 3665000);
}

TEST_F(PipelineTest, CreateDisplayState_AvgOdo) {
  TripState state      = createInitialState();
  state.speed.avg      = 18.3f;
  state.distance.total = 123.45f;

  GnssData   gnss = createGnssData(20.0f, Fix3D);
  SpGnssTime t    = {2024, 1, 1, 12, 0, 0, 0};

  DisplayState data = DisplayLogic::create(state, gnss, t, Mode::AVG_ODO);

  EXPECT_STREQ(data.modeSpeedLabel, "AVG");
  EXPECT_STREQ(data.modeTimeLabel, "Odo");
  EXPECT_FLOAT_EQ(data.mainValue, 18.3f);
  EXPECT_STREQ(data.mainUnit, "km/h");
  EXPECT_EQ(data.subType, DisplayState::SubType::Distance);
  EXPECT_FLOAT_EQ(data.subValue.distanceKm, 123.45f);
  EXPECT_STREQ(data.subUnit, "km");
}

TEST_F(PipelineTest, CreateDisplayState_MaxClk) {
  TripState state = createInitialState();
  state.speed.max = 45.2f;

  GnssData gnss            = createGnssData(20.0f, Fix3D);
  gnss.navData.time.year   = 2026;
  gnss.navData.time.hour   = 10;
  gnss.navData.time.minute = 30;

  DisplayState data = DisplayLogic::create(state, gnss, gnss.navData.time, Mode::MAX_CLK);

  EXPECT_STREQ(data.modeSpeedLabel, "MAX");
  EXPECT_STREQ(data.modeTimeLabel, "Clock");
  EXPECT_FLOAT_EQ(data.mainValue, 45.2f);
  EXPECT_EQ(data.subType, DisplayState::SubType::Clock);
  EXPECT_EQ(data.subValue.clockTime.hour, 19); // JST
  EXPECT_EQ(data.subValue.clockTime.minute, 30);
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
  // 10km を 1時間で移動 -> 10km/h
  EXPECT_FLOAT_EQ(TripLogic::calculateAverageSpeed(10.0f, 3600000), 10.0f);

  // 移動時間0 -> 0km/h
  EXPECT_FLOAT_EQ(TripLogic::calculateAverageSpeed(10.0f, 0), 0.0f);
}
