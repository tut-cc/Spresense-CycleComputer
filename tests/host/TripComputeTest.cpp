#include "../../src2/common/Config.h"
#include "../../src2/common/DataStructures.h"
#include "mocks/Arduino.h"
#include "mocks/GNSS.h"
#include <gtest/gtest.h>

// TripState コンストラクタによる計算ロジックのテスト

class TripComputeTest : public ::testing::Test {
protected:
  void SetUp() override { _mock_millis = 0; }

  // ヘルパー: 初期状態を作成
  TripState createInitialState() {
    TripState state;
    state.clearAllData();
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
    data.updated            = updated;
    return data;
  }

  // ヘルパー: Pause切替
  void togglePause(TripState &state) {
    state.status = state.isPaused() ? TripState::Status::Stopped : TripState::Status::Paused;
  }
};

// ========================================
// 基本機能のテスト
// ========================================

TEST_F(TripComputeTest, InitialState) {
  TripState state = createInitialState();
  EXPECT_FLOAT_EQ(state.speed.current, 0.0f);
  EXPECT_FLOAT_EQ(state.distance.total, 0.0f);
  EXPECT_EQ(state.status, TripState::Status::Stopped);
  EXPECT_EQ(state.time.moving, 0);
}

TEST_F(TripComputeTest, FirstUpdate) {
  TripState state = createInitialState();
  GnssData  gnss  = createGnssData(10.0f, Fix3D);
  state           = TripState(state, gnss, 1000);

  // 初回更新では lastUpdateTime のみ設定される
  EXPECT_EQ(state.lastUpdateTime, 1000);
}

TEST_F(TripComputeTest, UpdateStatusMoving) {
  TripState state = createInitialState();
  GnssData  gnss  = createGnssData(10.0f, Fix3D);

  // First update to set baseline
  state = TripState(state, gnss, 1000);

  // Second update to calculate dt and update status to Moving
  state = TripState(state, gnss, 2000);

  EXPECT_EQ(state.status, TripState::Status::Moving);
  EXPECT_NEAR(state.speed.current, 10.0f, 0.01f);
}

TEST_F(TripComputeTest, AverageSpeed) {
  TripState state = createInitialState();
  GnssData  gnss  = createGnssData(36.0f, Fix3D, 35.6812, 139.7671);

  state = TripState(state, gnss, 1000); // sets lastUpdateTime
  state = TripState(state, gnss, 2000); // status becomes Moving

  // Move 1000ms with 36km/h (10m/s) -> 10m
  state = TripState(state, gnss, 3000);

  state.updateAverageSpeed();

  EXPECT_GT(state.distance.trip, 0.0f);
  EXPECT_GT(state.time.moving, 0);
  EXPECT_GT(state.speed.avg, 0.0f);
}

TEST_F(TripComputeTest, GnssTimeout) {
  TripState state = createInitialState();
  GnssData  gnss  = createGnssData(10.0f, Fix3D);

  state = TripState(state, gnss, 1000);
  state = TripState(state, gnss, 2000);
  EXPECT_EQ(state.status, TripState::Status::Moving);

  // Timeout (dt exceeds SIGNAL_TIMEOUT_MS)
  gnss.updated = false;
  state        = TripState(state, gnss, 2000 + Config::Gnss::SIGNAL_TIMEOUT_MS + 100);
  EXPECT_EQ(state.status, TripState::Status::Stopped);
  EXPECT_FLOAT_EQ(state.speed.current, 0.0f);
}

TEST_F(TripComputeTest, GnssFixLost) {
  TripState state = createInitialState();
  GnssData  gnss  = createGnssData(10.0f, Fix3D);

  state = TripState(state, gnss, 1000);
  state = TripState(state, gnss, 2000);
  EXPECT_EQ(state.status, TripState::Status::Moving);

  // Lose fix
  gnss.updated            = true;
  gnss.navData.posFixMode = FixInvalid;
  state                   = TripState(state, gnss, 3000);
  EXPECT_EQ(state.status, TripState::Status::Stopped);
  EXPECT_FLOAT_EQ(state.speed.current, 0.0f);
}

TEST_F(TripComputeTest, GnssFix2D) {
  TripState state = createInitialState();
  GnssData  gnss  = createGnssData(10.0f, Fix2D); // Only 2D fix

  state = TripState(state, gnss, 1000);
  state = TripState(state, gnss, 2000);
  EXPECT_EQ(state.status, TripState::Status::Moving);
}

TEST_F(TripComputeTest, MinMovingSpeed) {
  TripState state = createInitialState();
  GnssData  gnss  = createGnssData(0.0f, Fix3D);

  // Just below threshold
  gnss.navData.velocity = (Config::Gnss::MIN_MOVING_SPEED_KMH - 0.0001f) / 3.6f;
  state                 = TripState(state, gnss, 1000);
  state                 = TripState(state, gnss, 2000);
  EXPECT_EQ(state.status, TripState::Status::Stopped);

  // Just above threshold
  gnss.navData.velocity = (Config::Gnss::MIN_MOVING_SPEED_KMH + 0.0001f) / 3.6f;
  state                 = TripState(state, gnss, 3000);
  EXPECT_EQ(state.status, TripState::Status::Moving);
}

// ========================================
// 経過時間の計算テスト
// ========================================

TEST_F(TripComputeTest, ElapsedTimeAccumulation) {
  TripState state = createInitialState();
  GnssData  gnss  = createGnssData(10.0f, Fix3D);

  state = TripState(state, gnss, 1000);
  state = TripState(state, gnss, 2000); // Moving になる
  EXPECT_EQ(state.time.elapsed, 1000);
  EXPECT_EQ(state.time.moving, 0);

  state = TripState(state, gnss, 3000); // 1000ms 加算
  EXPECT_EQ(state.time.elapsed, 2000);
  EXPECT_EQ(state.time.moving, 1000);
}

TEST_F(TripComputeTest, MovingTimeExcludesStopped) {
  TripState state = createInitialState();
  GnssData  gnss  = createGnssData(10.0f, Fix3D);

  state = TripState(state, gnss, 1000);
  state = TripState(state, gnss, 2000); // Status becomes Moving
  state = TripState(state, gnss, 3000); // Moving (1000ms added)
  EXPECT_EQ(state.time.moving, 1000);

  // Stop
  gnss.navData.velocity = 0.0f;
  state                 = TripState(state, gnss, 4000); // Last state was Moving
  state                 = TripState(state, gnss, 5000); // Last state was Stopped, so no add
  EXPECT_EQ(state.time.moving, 2000);
}

TEST_F(TripComputeTest, PausedTimeExcluded) {
  TripState state = createInitialState();
  GnssData  gnss  = createGnssData(10.0f, Fix3D);

  state = TripState(state, gnss, 1000);
  state = TripState(state, gnss, 2000); // Status becomes Moving
  state = TripState(state, gnss, 3000); // Moving (1000ms added)
  EXPECT_EQ(state.time.elapsed, 2000);
  EXPECT_EQ(state.time.moving, 1000);

  // Pause
  togglePause(state);
  EXPECT_EQ(state.status, TripState::Status::Paused);

  state = TripState(state, gnss, 4000); // Last status was Paused
  EXPECT_EQ(state.time.elapsed, 2000);  // No change
  EXPECT_EQ(state.time.moving, 1000);   // No change
}

// ========================================
// 最高速度のテスト
// ========================================

TEST_F(TripComputeTest, MaxSpeedTracking) {
  TripState state = createInitialState();
  GnssData  gnss  = createGnssData(10.0f, Fix3D);

  state = TripState(state, gnss, 1000);
  state = TripState(state, gnss, 2000);
  EXPECT_NEAR(state.speed.max, 10.0f, 0.01f);

  // Increase speed
  gnss.navData.velocity = 20.0f / 3.6f;
  state                 = TripState(state, gnss, 3000);
  EXPECT_NEAR(state.speed.max, 20.0f, 0.01f);

  // Decrease speed
  gnss.navData.velocity = 5.0f / 3.6f;
  state                 = TripState(state, gnss, 4000);
  EXPECT_NEAR(state.speed.max, 20.0f, 0.01f);
}

// ========================================
// Pause状態での距離計算テスト
// ========================================

TEST_F(TripComputeTest, PausedDoesNotAccumulateTripDistance) {
  TripState state = createInitialState();
  GnssData  gnss  = createGnssData(10.0f, Fix3D, 35.0, 135.0);

  state = TripState(state, gnss, 1000);
  state = TripState(state, gnss, 2000);

  // Move
  state           = TripState(state, gnss, 3000);
  float tripDist  = state.distance.trip;
  float totalDist = state.distance.total;

  // Pause
  togglePause(state);

  // Move while paused
  state = TripState(state, gnss, 4000);

  // tripDistance and totalKm should NOT change while paused
  EXPECT_FLOAT_EQ(state.distance.trip, tripDist);
  EXPECT_FLOAT_EQ(state.distance.total, totalDist);
}
