#include "../../src2/Pipeline.h"
#include "../../src2/TripCompute.h"
#include "mocks/Arduino.h"
#include "mocks/GNSS.h"
#include <gtest/gtest.h>

// --- Pipeline Tests ---

class PipelineTest : public ::testing::Test {
protected:
  void SetUp() override {
    _mock_millis = 0;
  }

  // ヘルパー: 初期状態を作成
  TripStateData createInitialState() {
    TripStateData state;
    state.currentSpeed   = 0.0f;
    state.status         = TripStateData::Status::Stopped;
    state.totalElapsedMs = 0;
    state.maxSpeed       = 0.0f;
    state.totalKm        = 0.0f;
    state.tripDistance   = 0.0f;
    state.totalMovingMs  = 0;
    state.avgSpeed       = 0.0f;
    state.lastUpdateTime = 0;
    state.updateStatus   = UpdateStatus::NoChange;
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
  EXPECT_EQ(Pipeline::determineResetType(Input::Event::RESET_LONG, Mode::ID::SPD_TIM),
            Pipeline::ResetType::AllWithStorage);

  // RESET + SPD_TIM -> Trip
  EXPECT_EQ(Pipeline::determineResetType(Input::Event::RESET, Mode::ID::SPD_TIM),
            Pipeline::ResetType::Trip);

  // RESET + AVG_ODO -> All
  EXPECT_EQ(Pipeline::determineResetType(Input::Event::RESET, Mode::ID::AVG_ODO),
            Pipeline::ResetType::All);

  // RESET + MAX_CLK -> MaxSpeed
  EXPECT_EQ(Pipeline::determineResetType(Input::Event::RESET, Mode::ID::MAX_CLK),
            Pipeline::ResetType::MaxSpeed);

  // その他 -> None
  EXPECT_EQ(Pipeline::determineResetType(Input::Event::NONE, Mode::ID::SPD_TIM),
            Pipeline::ResetType::None);
}

TEST_F(PipelineTest, ApplyReset_Trip) {
  TripStateData state  = createInitialState();
  state.totalElapsedMs = 5000;
  state.tripDistance   = 10.5f;
  state.totalKm        = 100.0f;
  state.maxSpeed       = 50.0f;

  TripStateData newState = Pipeline::applyReset(state, Pipeline::ResetType::Trip);

  // トリップデータのみリセット
  EXPECT_EQ(newState.totalElapsedMs, 0);
  EXPECT_FLOAT_EQ(newState.tripDistance, 0.0f);
  EXPECT_EQ(newState.status, TripStateData::Status::Stopped);

  // 累積データは保持
  EXPECT_FLOAT_EQ(newState.totalKm, 100.0f);
  EXPECT_FLOAT_EQ(newState.maxSpeed, 50.0f);

  EXPECT_EQ(newState.updateStatus, UpdateStatus::ForceUpdate);
}

TEST_F(PipelineTest, ApplyReset_MaxSpeed) {
  TripStateData state = createInitialState();
  state.maxSpeed      = 50.0f;
  state.tripDistance  = 10.5f;

  TripStateData newState = Pipeline::applyReset(state, Pipeline::ResetType::MaxSpeed);

  // 最高速度のみリセット
  EXPECT_FLOAT_EQ(newState.maxSpeed, 0.0f);

  // 他のデータは保持
  EXPECT_FLOAT_EQ(newState.tripDistance, 10.5f);

  EXPECT_EQ(newState.updateStatus, UpdateStatus::ForceUpdate);
}

TEST_F(PipelineTest, ApplyReset_All) {
  TripStateData state  = createInitialState();
  state.totalElapsedMs = 5000;
  state.tripDistance   = 10.5f;
  state.totalKm        = 100.0f;
  state.maxSpeed       = 50.0f;

  TripStateData newState = Pipeline::applyReset(state, Pipeline::ResetType::All);

  // 全データリセット
  EXPECT_EQ(newState.totalElapsedMs, 0);
  EXPECT_FLOAT_EQ(newState.tripDistance, 0.0f);
  EXPECT_FLOAT_EQ(newState.totalKm, 0.0f);
  EXPECT_FLOAT_EQ(newState.maxSpeed, 0.0f);
  EXPECT_EQ(newState.status, TripStateData::Status::Stopped);

  EXPECT_EQ(newState.updateStatus, UpdateStatus::ForceUpdate);
}

TEST_F(PipelineTest, ApplyPause) {
  TripStateData state = createInitialState();
  state.status        = TripStateData::Status::Stopped;

  // Stopped -> Paused
  TripStateData newState = Pipeline::applyPause(state);
  EXPECT_EQ(newState.status, TripStateData::Status::Paused);
  EXPECT_EQ(newState.updateStatus, UpdateStatus::ForceUpdate);

  // Paused -> Stopped
  newState = Pipeline::applyPause(newState);
  EXPECT_EQ(newState.status, TripStateData::Status::Stopped);
}

TEST_F(PipelineTest, SwitchMode) {
  // SELECT -> 次のモード
  EXPECT_EQ(Pipeline::switchMode(Mode::ID::SPD_TIM, Input::Event::SELECT), Mode::ID::AVG_ODO);
  EXPECT_EQ(Pipeline::switchMode(Mode::ID::AVG_ODO, Input::Event::SELECT), Mode::ID::MAX_CLK);
  EXPECT_EQ(Pipeline::switchMode(Mode::ID::MAX_CLK, Input::Event::SELECT), Mode::ID::SPD_TIM);

  // その他 -> 変更なし
  EXPECT_EQ(Pipeline::switchMode(Mode::ID::SPD_TIM, Input::Event::NONE), Mode::ID::SPD_TIM);
}

TEST_F(PipelineTest, HandleUserInput_Pause) {
  TripStateData state = createInitialState();

  auto result =
      Pipeline::handleUserInput<TripStateData>(state, Mode::ID::SPD_TIM, Input::Event::PAUSE);

  EXPECT_EQ(result.newState.status, TripStateData::Status::Paused);
  EXPECT_EQ(result.newMode, Mode::ID::SPD_TIM);
  EXPECT_FALSE(result.shouldClearStorage);
}

TEST_F(PipelineTest, HandleUserInput_ResetLong) {
  TripStateData state = createInitialState();
  state.totalKm       = 100.0f;

  auto result =
      Pipeline::handleUserInput<TripStateData>(state, Mode::ID::SPD_TIM, Input::Event::RESET_LONG);

  EXPECT_FLOAT_EQ(result.newState.totalKm, 0.0f);
  EXPECT_TRUE(result.shouldClearStorage);
}

// ========================================
// 表示データ生成のテスト
// ========================================

TEST_F(PipelineTest, CreateDisplayData_SpdTim) {
  TripStateData state  = createInitialState();
  state.currentSpeed   = 25.5f;
  state.totalElapsedMs = 3665000; // 1:01:05

  GnssData gnss = createGnssData(25.5f, Fix3D);

  DisplayData data = Pipeline::createDisplayData(state, gnss, Mode::ID::SPD_TIM);

  EXPECT_STREQ(data.modeSpeedLabel, "SPD");
  EXPECT_STREQ(data.modeTimeLabel, "Time");
  EXPECT_FLOAT_EQ(data.mainValue, 25.5f);
  EXPECT_STREQ(data.mainUnit, "km/h");
  EXPECT_EQ(data.subType, DisplayData::SubType::Duration);
  EXPECT_EQ(data.subValue.durationMs, 3665000);
}

TEST_F(PipelineTest, CreateDisplayData_AvgOdo) {
  TripStateData state = createInitialState();
  state.avgSpeed      = 18.3f;
  state.totalKm       = 123.45f;

  GnssData gnss = createGnssData(20.0f, Fix3D);

  DisplayData data = Pipeline::createDisplayData(state, gnss, Mode::ID::AVG_ODO);

  EXPECT_STREQ(data.modeSpeedLabel, "AVG");
  EXPECT_STREQ(data.modeTimeLabel, "Odo");
  EXPECT_FLOAT_EQ(data.mainValue, 18.3f);
  EXPECT_STREQ(data.mainUnit, "km/h");
  EXPECT_EQ(data.subType, DisplayData::SubType::Distance);
  EXPECT_FLOAT_EQ(data.subValue.distanceKm, 123.45f);
  EXPECT_STREQ(data.subUnit, "km");
}

TEST_F(PipelineTest, CreateDisplayData_MaxClk) {
  TripStateData state = createInitialState();
  state.maxSpeed      = 45.2f;

  GnssData gnss            = createGnssData(20.0f, Fix3D);
  gnss.navData.time.year   = 2026;
  gnss.navData.time.hour   = 10;
  gnss.navData.time.minute = 30;

  DisplayData data = Pipeline::createDisplayData(state, gnss, Mode::ID::MAX_CLK);

  EXPECT_STREQ(data.modeSpeedLabel, "MAX");
  EXPECT_STREQ(data.modeTimeLabel, "Clock");
  EXPECT_FLOAT_EQ(data.mainValue, 45.2f);
  EXPECT_EQ(data.subType, DisplayData::SubType::Clock);
  EXPECT_EQ(data.subValue.clockTime.hour, 19); // JST
  EXPECT_EQ(data.subValue.clockTime.minute, 30);
}

// ========================================
// 永続化データ生成のテスト
// ========================================

TEST_F(PipelineTest, CreatePersistentData) {
  TripStateData state = createInitialState();
  state.totalKm       = 123.45f;
  state.tripDistance  = 10.5f;
  state.totalMovingMs = 3600000;
  state.maxSpeed      = 45.2f;
  state.updateStatus  = UpdateStatus::Updated;

  PersistentData data = Pipeline::createPersistentData(state, 4.2f);

  EXPECT_FLOAT_EQ(data.totalDistance, 123.45f);
  EXPECT_FLOAT_EQ(data.tripDistance, 10.5f);
  EXPECT_EQ(data.movingTimeMs, 3600000);
  EXPECT_FLOAT_EQ(data.maxSpeed, 45.2f);
  EXPECT_FLOAT_EQ(data.voltage, 4.2f);
  EXPECT_EQ(data.updateStatus, UpdateStatus::Updated);
}

// ========================================
// Trip計算のヘルパー関数テスト
// ========================================

TEST_F(PipelineTest, CalculateRawKmh) {
  EXPECT_FLOAT_EQ(Pipeline::calculateRawKmh(10.0f / 3.6f), 10.0f);
}

TEST_F(PipelineTest, HasFix) {
  EXPECT_TRUE(Pipeline::hasFix(Fix2D));
  EXPECT_TRUE(Pipeline::hasFix(Fix3D));
  EXPECT_FALSE(Pipeline::hasFix(FixInvalid));
}

TEST_F(PipelineTest, IsMoving) {
  EXPECT_TRUE(Pipeline::isMoving(true, 10.0f));
  EXPECT_FALSE(Pipeline::isMoving(false, 10.0f));
  EXPECT_FALSE(Pipeline::isMoving(true, 0.0f));
}

TEST_F(PipelineTest, CalculateAverageSpeed) {
  // 10km を 1時間で移動 -> 10km/h
  EXPECT_FLOAT_EQ(Pipeline::calculateAverageSpeed(10.0f, 3600000), 10.0f);

  // 移動時間0 -> 0km/h
  EXPECT_FLOAT_EQ(Pipeline::calculateAverageSpeed(10.0f, 0), 0.0f);
}

TEST_F(PipelineTest, IsValidCoordinate) {
  EXPECT_TRUE(Pipeline::isValidCoordinate(35.0f, 135.0f));
  EXPECT_FALSE(Pipeline::isValidCoordinate(0.0f, 0.0f));
  EXPECT_TRUE(Pipeline::isValidCoordinate(0.1f, 0.0f));
}
