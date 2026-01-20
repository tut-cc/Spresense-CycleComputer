#include "../../src2/domain/TripLogic.h"
#include "mocks/Arduino.h"
#include "mocks/GNSS.h"
#include <gtest/gtest.h>

// computeTrip関数の統合テスト
// 既存のTripTestと同等のテストケースを実装

class TripComputeTest : public ::testing::Test {
protected:
  void SetUp() override { _mock_millis = 0; }

  // ヘルパー: 初期状態を作成
  TripState createInitialState() {
    TripState state;
    state.resetAll();
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

  // ヘルパー: Pause切替（InputLogicの代わり）
  void togglePause(TripState &state) {
    state.status =
        state.isPaused() ? TripStateBase::Status::Stopped : TripStateBase::Status::Paused;
    state.forceUpdate();
  }
};

// ========================================
// 基本機能のテスト
// ========================================

TEST_F(TripComputeTest, InitialState) {
  TripState state = createInitialState();
  EXPECT_FLOAT_EQ(state.speed.current, 0.0f);
  EXPECT_FLOAT_EQ(state.distance.total, 0.0f);
  EXPECT_EQ(state.status, TripStateBase::Status::Stopped);
  EXPECT_EQ(state.time.moving, 0);
}

TEST_F(TripComputeTest, FirstUpdate) {
  TripState state = createInitialState();
  GnssData  gnss  = createGnssData(10.0f, Fix3D);
  TripLogic::computeTrip(state, gnss, 1000);

  // 初回更新では lastUpdateTime のみ設定される
  EXPECT_EQ(state.lastUpdateTime, 1000);
  EXPECT_EQ(state.updateStatus, UpdateStatus::Updated);
}

TEST_F(TripComputeTest, UpdateStatusMoving) {
  TripState state = createInitialState();
  GnssData  gnss  = createGnssData(10.0f, Fix3D);

  // First update to set baseline
  TripLogic::computeTrip(state, gnss, 1000);

  // Second update to calculate dt and update status to Moving
  TripLogic::computeTrip(state, gnss, 2000);

  EXPECT_EQ(state.status, TripStateBase::Status::Moving);
  EXPECT_NEAR(state.speed.current, 10.0f, 0.01f);
}

TEST_F(TripComputeTest, AverageSpeed) {
  TripState state = createInitialState();
  GnssData  gnss  = createGnssData(36.0f, Fix3D, 35.6812, 139.7671);

  TripLogic::computeTrip(state, gnss, 1000); // sets lastUpdateTime
  TripLogic::computeTrip(state, gnss, 2000); // sets hasLastCoord, status becomes Moving

  // Move to another coordinate (approx 110m away)
  gnss.navData.latitude = 35.6822;
  TripLogic::computeTrip(state, gnss, 3000); // tripDistance increments, time.moving increments
  TripLogic::computeTrip(state, gnss, 4000); // additional stats update

  EXPECT_GT(state.distance.trip, 0.0f);
  EXPECT_GT(state.time.moving, 0);
  EXPECT_GT(state.speed.avg, 0.0f);
}

TEST_F(TripComputeTest, GnssTimeout) {
  TripState state = createInitialState();
  GnssData  gnss  = createGnssData(10.0f, Fix3D);

  TripLogic::computeTrip(state, gnss, 1000);
  TripLogic::computeTrip(state, gnss, 2000);
  EXPECT_EQ(state.status, TripStateBase::Status::Moving);

  // Timeout
  gnss.status = UpdateStatus::NoChange;
  TripLogic::computeTrip(state, gnss, 2000 + TripLogic::SIGNAL_TIMEOUT_MS + 100);
  EXPECT_EQ(state.status, TripStateBase::Status::Stopped);
  EXPECT_FLOAT_EQ(state.speed.current, 0.0f);
}

TEST_F(TripComputeTest, GnssFixLost) {
  TripState state = createInitialState();
  GnssData  gnss  = createGnssData(10.0f, Fix3D);

  TripLogic::computeTrip(state, gnss, 1000);
  TripLogic::computeTrip(state, gnss, 2000);
  EXPECT_EQ(state.status, TripStateBase::Status::Moving);

  // Lose fix
  gnss.navData.posFixMode = FixInvalid;
  TripLogic::computeTrip(state, gnss, 3000);
  EXPECT_EQ(state.status, TripStateBase::Status::Stopped);
  EXPECT_FLOAT_EQ(state.speed.current, 0.0f);
}

TEST_F(TripComputeTest, GnssFix2D) {
  TripState state = createInitialState();
  GnssData  gnss  = createGnssData(10.0f, Fix2D); // Only 2D fix

  TripLogic::computeTrip(state, gnss, 1000);
  TripLogic::computeTrip(state, gnss, 2000);
  EXPECT_EQ(state.status, TripStateBase::Status::Moving);
}

TEST_F(TripComputeTest, MinMovingSpeed) {
  TripState state = createInitialState();
  GnssData  gnss  = createGnssData(0.0f, Fix3D);

  // Just below threshold
  gnss.navData.velocity = (TripLogic::MIN_MOVING_SPEED_KMH - 0.0001f) / 3.6f;
  TripLogic::computeTrip(state, gnss, 1000);
  TripLogic::computeTrip(state, gnss, 2000);
  EXPECT_EQ(state.status, TripStateBase::Status::Stopped);

  // Just above threshold
  gnss.navData.velocity = (TripLogic::MIN_MOVING_SPEED_KMH + 0.0001f) / 3.6f;
  TripLogic::computeTrip(state, gnss, 3000);
  EXPECT_EQ(state.status, TripStateBase::Status::Moving);
}

// ========================================
// 経過時間の計算テスト
// ========================================

TEST_F(TripComputeTest, ElapsedTimeAccumulation) {
  TripState state = createInitialState();
  GnssData  gnss  = createGnssData(10.0f, Fix3D);

  TripLogic::computeTrip(state, gnss, 1000);
  TripLogic::computeTrip(state, gnss, 2000); // Moving になるが、加算は次から
  EXPECT_EQ(state.time.elapsed, 1000);
  EXPECT_EQ(state.time.moving, 0);

  TripLogic::computeTrip(state, gnss, 3000); // ここで Moving として 1000ms 加算される
  EXPECT_EQ(state.time.elapsed, 2000);
  EXPECT_EQ(state.time.moving, 1000);
}

TEST_F(TripComputeTest, MovingTimeExcludesStopped) {
  TripState state = createInitialState();
  GnssData  gnss  = createGnssData(10.0f, Fix3D);

  TripLogic::computeTrip(state, gnss, 1000);
  TripLogic::computeTrip(state, gnss, 2000); // Status becomes Moving
  TripLogic::computeTrip(state, gnss, 3000); // Moving (1000ms added)
  EXPECT_EQ(state.time.moving, 1000);

  // Stop
  gnss.navData.velocity = 0.0f;
  TripLogic::computeTrip(state, gnss, 4000); // Still 1000ms (last state was Moving)
  TripLogic::computeTrip(state, gnss, 5000); // Last state was Stopped, so no add
  EXPECT_EQ(state.time.moving, 2000);        // (3000-4000) was Moving, (4000-5000) was Stopped
}

TEST_F(TripComputeTest, PausedTimeExcluded) {
  TripState state = createInitialState();
  GnssData  gnss  = createGnssData(10.0f, Fix3D);

  TripLogic::computeTrip(state, gnss, 1000);
  TripLogic::computeTrip(state, gnss, 2000); // Status becomes Moving
  TripLogic::computeTrip(state, gnss, 3000); // Moving (1000ms added)
  EXPECT_EQ(state.time.elapsed, 2000);       // (1000-2000) Stopped, (2000-3000) Moving
  EXPECT_EQ(state.time.moving, 1000);        // (2000-3000) Moving

  // Pause
  togglePause(state);
  EXPECT_EQ(state.status, TripStateBase::Status::Paused);

  TripLogic::computeTrip(state, gnss, 4000); // Last status was Paused
  EXPECT_EQ(state.time.elapsed, 2000);       // No change
  EXPECT_EQ(state.time.moving, 1000);        // No change
}

// ========================================
// 最高速度のテスト
// ========================================

TEST_F(TripComputeTest, MaxSpeedTracking) {
  TripState state = createInitialState();
  GnssData  gnss  = createGnssData(10.0f, Fix3D);

  TripLogic::computeTrip(state, gnss, 1000);
  TripLogic::computeTrip(state, gnss, 2000);
  EXPECT_NEAR(state.speed.max, 10.0f, 0.01f);

  // Increase speed
  gnss.navData.velocity = 20.0f / 3.6f;
  TripLogic::computeTrip(state, gnss, 3000);
  EXPECT_NEAR(state.speed.max, 20.0f, 0.01f);

  // Decrease speed (max should not change)
  gnss.navData.velocity = 5.0f / 3.6f;
  TripLogic::computeTrip(state, gnss, 4000);
  EXPECT_NEAR(state.speed.max, 20.0f, 0.01f);
}

// ========================================
// Pause状態での距離計算テスト
// ========================================

TEST_F(TripComputeTest, PausedDoesNotAccumulateTripDistance) {
  TripState state = createInitialState();
  GnssData  gnss  = createGnssData(10.0f, Fix3D, 35.0, 135.0);

  TripLogic::computeTrip(state, gnss, 1000);
  TripLogic::computeTrip(state, gnss, 2000); // hasLastCoord set

  // Move
  gnss.navData.latitude = 35.001;
  TripLogic::computeTrip(state, gnss, 3000);
  float tripDist  = state.distance.trip;
  float totalDist = state.distance.total;

  // Pause
  togglePause(state);

  // Move while paused
  // Just advancing time with velocity
  TripLogic::computeTrip(state, gnss, 4000);

  // tripDistance and totalKm should NOT change while paused
  EXPECT_FLOAT_EQ(state.distance.trip, tripDist);
  EXPECT_FLOAT_EQ(state.distance.total, totalDist);
}
