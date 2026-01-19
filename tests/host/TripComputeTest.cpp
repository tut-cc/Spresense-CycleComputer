#include "../../src2/TripCompute.h"
#include "../../src2/Pipeline.h"
#include "mocks/Arduino.h"
#include "mocks/GNSS.h"
#include <gtest/gtest.h>

// computeTrip関数の統合テスト
// 既存のTripTestと同等のテストケースを実装

class TripComputeTest : public ::testing::Test {
protected:
  void SetUp() override {
    _mock_millis = 0;
  }

  // ヘルパー: 初期状態を作成
  TripStateDataEx createInitialState() {
    TripStateDataEx state;
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
    state.lastLat        = 0.0f;
    state.lastLon        = 0.0f;
    state.hasLastCoord   = false;
    return state;
  }

  // ヘルパー: GNSSデータを作成
  GnssData createGnssData(float velocityKmh, SpFixMode fixMode, float lat = 35.6812,
                          float lon = 139.7671, bool updated = true) {
    GnssData data;
    data.navData.velocity   = velocityKmh / 3.6f;
    data.navData.posFixMode = fixMode;
    data.navData.latitude   = lat;
    data.navData.longitude  = lon;
    data.timestamp          = millis();
    data.status             = updated ? UpdateStatus::Updated : UpdateStatus::NoChange;
    return data;
  }
};

// ========================================
// 基本機能のテスト
// ========================================

TEST_F(TripComputeTest, InitialState) {
  TripStateDataEx state = createInitialState();
  EXPECT_FLOAT_EQ(state.currentSpeed, 0.0f);
  EXPECT_FLOAT_EQ(state.totalKm, 0.0f);
  EXPECT_EQ(state.status, TripStateData::Status::Stopped);
  EXPECT_EQ(state.totalMovingMs, 0);
}

TEST_F(TripComputeTest, FirstUpdate) {
  TripStateDataEx state    = createInitialState();
  GnssData        gnss     = createGnssData(10.0f, Fix3D);
  TripStateDataEx newState = Pipeline::computeTrip(state, gnss, 1000);

  // 初回更新では lastUpdateTime のみ設定される
  EXPECT_EQ(newState.lastUpdateTime, 1000);
  EXPECT_EQ(newState.updateStatus, UpdateStatus::Updated);
}

TEST_F(TripComputeTest, UpdateStatusMoving) {
  TripStateDataEx state = createInitialState();
  GnssData        gnss  = createGnssData(10.0f, Fix3D);

  // First update to set baseline
  state = Pipeline::computeTrip(state, gnss, 1000);

  // Second update to calculate dt and update status to Moving
  state = Pipeline::computeTrip(state, gnss, 2000);

  EXPECT_EQ(state.status, TripStateData::Status::Moving);
  EXPECT_NEAR(state.currentSpeed, 10.0f, 0.01f);
}

TEST_F(TripComputeTest, AverageSpeed) {
  TripStateDataEx state = createInitialState();
  GnssData        gnss  = createGnssData(36.0f, Fix3D, 35.6812, 139.7671);

  state = Pipeline::computeTrip(state, gnss, 1000); // sets lastUpdateTime

  state = Pipeline::computeTrip(state, gnss, 2000); // sets hasLastCoord, status becomes Moving

  // Move to another coordinate (approx 110m away)
  gnss.navData.latitude = 35.6822;
  state =
      Pipeline::computeTrip(state, gnss, 3000); // tripDistance increments, totalMovingMs increments

  state = Pipeline::computeTrip(state, gnss, 4000); // additional stats update

  EXPECT_GT(state.tripDistance, 0.0f);
  EXPECT_GT(state.totalMovingMs, 0);
  EXPECT_GT(state.avgSpeed, 0.0f);
}

TEST_F(TripComputeTest, GnssTimeout) {
  TripStateDataEx state = createInitialState();
  GnssData        gnss  = createGnssData(10.0f, Fix3D);

  state = Pipeline::computeTrip(state, gnss, 1000);
  state = Pipeline::computeTrip(state, gnss, 2000);
  EXPECT_EQ(state.status, TripStateData::Status::Moving);

  // Timeout
  gnss.status = UpdateStatus::NoChange;
  state       = Pipeline::computeTrip(state, gnss, 2000 + Pipeline::SIGNAL_TIMEOUT_MS + 100);
  EXPECT_EQ(state.status, TripStateData::Status::Stopped);
  EXPECT_FLOAT_EQ(state.currentSpeed, 0.0f);
}

TEST_F(TripComputeTest, InvalidCoordinate) {
  TripStateDataEx state = createInitialState();
  GnssData        gnss  = createGnssData(10.0f, Fix3D, 35.0, 135.0);

  state             = Pipeline::computeTrip(state, gnss, 1000);
  state             = Pipeline::computeTrip(state, gnss, 2000);
  float initialDist = state.totalKm;

  // Update with (0,0)
  gnss.navData.latitude  = 0.0;
  gnss.navData.longitude = 0.0;
  state                  = Pipeline::computeTrip(state, gnss, 3000);
  EXPECT_FLOAT_EQ(state.totalKm, initialDist);
}

TEST_F(TripComputeTest, ExtremeDistanceJump) {
  TripStateDataEx state = createInitialState();
  GnssData        gnss  = createGnssData(10.0f, Fix3D, 35.0, 135.0);

  state             = Pipeline::computeTrip(state, gnss, 1000);
  state             = Pipeline::computeTrip(state, gnss, 2000);
  float initialDist = state.totalKm;

  // Jump to another country (too far)
  gnss.navData.latitude  = 40.0;
  gnss.navData.longitude = 140.0;
  state                  = Pipeline::computeTrip(state, gnss, 3000);
  EXPECT_FLOAT_EQ(state.totalKm, initialDist);
}

TEST_F(TripComputeTest, GnssFixLost) {
  TripStateDataEx state = createInitialState();
  GnssData        gnss  = createGnssData(10.0f, Fix3D);

  state = Pipeline::computeTrip(state, gnss, 1000);
  state = Pipeline::computeTrip(state, gnss, 2000);
  EXPECT_EQ(state.status, TripStateData::Status::Moving);

  // Lose fix
  gnss.navData.posFixMode = FixInvalid;
  state                   = Pipeline::computeTrip(state, gnss, 3000);
  EXPECT_EQ(state.status, TripStateData::Status::Stopped);
  EXPECT_FLOAT_EQ(state.currentSpeed, 0.0f);
}

TEST_F(TripComputeTest, GnssJitter) {
  TripStateDataEx state = createInitialState();
  GnssData        gnss  = createGnssData(10.0f, Fix3D, 35.0, 135.0);

  state             = Pipeline::computeTrip(state, gnss, 1000);
  state             = Pipeline::computeTrip(state, gnss, 2000);
  float initialDist = state.totalKm;

  // Tiny movement (below MIN_DELTA = 2m)
  // 1 meter is approx 0.000009 degrees
  gnss.navData.latitude += 0.000005; // ~0.5 meters
  state = Pipeline::computeTrip(state, gnss, 3000);
  EXPECT_FLOAT_EQ(state.totalKm, initialDist);
}

TEST_F(TripComputeTest, GnssFix2D) {
  TripStateDataEx state = createInitialState();
  GnssData        gnss  = createGnssData(10.0f, Fix2D); // Only 2D fix

  state = Pipeline::computeTrip(state, gnss, 1000);
  state = Pipeline::computeTrip(state, gnss, 2000);
  EXPECT_EQ(state.status, TripStateData::Status::Moving);
}

TEST_F(TripComputeTest, MinMovingSpeed) {
  TripStateDataEx state = createInitialState();
  GnssData        gnss  = createGnssData(0.0f, Fix3D);

  // Just below threshold
  gnss.navData.velocity = (Pipeline::MIN_MOVING_SPEED_KMH - 0.0001f) / 3.6f;
  state                 = Pipeline::computeTrip(state, gnss, 1000);
  state                 = Pipeline::computeTrip(state, gnss, 2000);
  EXPECT_EQ(state.status, TripStateData::Status::Stopped);

  // Just above threshold
  gnss.navData.velocity = (Pipeline::MIN_MOVING_SPEED_KMH + 0.0001f) / 3.6f;
  state                 = Pipeline::computeTrip(state, gnss, 3000);
  EXPECT_EQ(state.status, TripStateData::Status::Moving);
}

TEST_F(TripComputeTest, DistanceDeltaLimits) {
  TripStateDataEx state = createInitialState();
  GnssData        gnss  = createGnssData(10.0f, Fix3D, 35.0, 135.0);

  state = Pipeline::computeTrip(state, gnss, 1000);
  state = Pipeline::computeTrip(state, gnss, 2000); // hasLastCoord set

  float initialDist = state.totalKm;

  // Change coordinate by approx 3.3 meters (above 2m MIN_DELTA)
  gnss.navData.latitude += 0.00003;
  state = Pipeline::computeTrip(state, gnss, 3000);
  EXPECT_GT(state.totalKm, initialDist);
}

// ========================================
// 経過時間の計算テスト
// ========================================

TEST_F(TripComputeTest, ElapsedTimeAccumulation) {
  TripStateDataEx state = createInitialState();
  GnssData        gnss  = createGnssData(10.0f, Fix3D);

  state = Pipeline::computeTrip(state, gnss, 1000);
  state = Pipeline::computeTrip(state, gnss, 2000); // Moving になるが、加算は次から
  EXPECT_EQ(state.totalElapsedMs, 1000);
  EXPECT_EQ(state.totalMovingMs, 0);

  state = Pipeline::computeTrip(state, gnss, 3000); // ここで Moving として 1000ms 加算される
  EXPECT_EQ(state.totalElapsedMs, 2000);
  EXPECT_EQ(state.totalMovingMs, 1000);
}

TEST_F(TripComputeTest, MovingTimeExcludesStopped) {
  TripStateDataEx state = createInitialState();
  GnssData        gnss  = createGnssData(10.0f, Fix3D);

  state = Pipeline::computeTrip(state, gnss, 1000);
  state = Pipeline::computeTrip(state, gnss, 2000); // Status becomes Moving
  state = Pipeline::computeTrip(state, gnss, 3000); // Moving (1000ms added)
  EXPECT_EQ(state.totalMovingMs, 1000);

  // Stop
  gnss.navData.velocity = 0.0f;
  state = Pipeline::computeTrip(state, gnss, 4000); // Still 1000ms (last state was Moving)
  state = Pipeline::computeTrip(state, gnss, 5000); // Last state was Stopped, so no add
  EXPECT_EQ(state.totalMovingMs, 2000); // (3000-4000) was Moving, (4000-5000) was Stopped
}

TEST_F(TripComputeTest, PausedTimeExcluded) {
  TripStateDataEx state = createInitialState();
  GnssData        gnss  = createGnssData(10.0f, Fix3D);

  state = Pipeline::computeTrip(state, gnss, 1000);
  state = Pipeline::computeTrip(state, gnss, 2000); // Status becomes Moving
  state = Pipeline::computeTrip(state, gnss, 3000); // Moving (1000ms added)
  EXPECT_EQ(state.totalElapsedMs, 2000);            // (1000-2000) Stopped, (2000-3000) Moving
  EXPECT_EQ(state.totalMovingMs, 1000);             // (2000-3000) Moving

  // Pause
  state = Pipeline::applyPause(state);
  EXPECT_EQ(state.status, TripStateData::Status::Paused);

  state = Pipeline::computeTrip(state, gnss, 4000); // Last status was Paused
  EXPECT_EQ(state.totalElapsedMs, 2000);            // No change
  EXPECT_EQ(state.totalMovingMs, 1000);             // No change
}

// ========================================
// 最高速度のテスト
// ========================================

TEST_F(TripComputeTest, MaxSpeedTracking) {
  TripStateDataEx state = createInitialState();
  GnssData        gnss  = createGnssData(10.0f, Fix3D);

  state = Pipeline::computeTrip(state, gnss, 1000);
  state = Pipeline::computeTrip(state, gnss, 2000);
  EXPECT_NEAR(state.maxSpeed, 10.0f, 0.01f);

  // Increase speed
  gnss.navData.velocity = 20.0f / 3.6f;
  state                 = Pipeline::computeTrip(state, gnss, 3000);
  EXPECT_NEAR(state.maxSpeed, 20.0f, 0.01f);

  // Decrease speed (max should not change)
  gnss.navData.velocity = 5.0f / 3.6f;
  state                 = Pipeline::computeTrip(state, gnss, 4000);
  EXPECT_NEAR(state.maxSpeed, 20.0f, 0.01f);
}

// ========================================
// Pause状態での距離計算テスト
// ========================================

TEST_F(TripComputeTest, PausedDoesNotAccumulateTripDistance) {
  TripStateDataEx state = createInitialState();
  GnssData        gnss  = createGnssData(10.0f, Fix3D, 35.0, 135.0);

  state = Pipeline::computeTrip(state, gnss, 1000);
  state = Pipeline::computeTrip(state, gnss, 2000); // hasLastCoord set

  // Move
  gnss.navData.latitude = 35.001;
  state                 = Pipeline::computeTrip(state, gnss, 3000);
  float tripDist        = state.tripDistance;
  float totalDist       = state.totalKm;

  // Pause
  state = Pipeline::applyPause(state);

  // Move while paused
  gnss.navData.latitude = 35.002;
  state                 = Pipeline::computeTrip(state, gnss, 4000);

  // tripDistance should not change, but totalKm should
  EXPECT_FLOAT_EQ(state.tripDistance, tripDist);
  EXPECT_GT(state.totalKm, totalDist);
}

// ========================================
// 平均速度の定期更新テスト
// ========================================

TEST_F(TripComputeTest, AverageSpeedPeriodicUpdate) {
  TripStateDataEx state = createInitialState();
  GnssData        gnss  = createGnssData(10.0f, Fix3D, 35.0, 135.0);

  state = Pipeline::computeTrip(state, gnss, 1000);
  state = Pipeline::computeTrip(state, gnss, 2000);

  // Move to accumulate distance
  gnss.navData.latitude = 35.001;
  state                 = Pipeline::computeTrip(state, gnss, 3000);
  float avgSpeed        = state.avgSpeed;

  // GNSS未更新でも1秒経過で平均速度が更新される
  gnss.status = UpdateStatus::NoChange;
  state       = Pipeline::computeTrip(state, gnss, 4000);

  // 移動時間が増えたので平均速度は下がる
  EXPECT_LT(state.avgSpeed, avgSpeed);
}
