#include "../../src2/Pipeline.h"
#include "../../src2/TripCompute.h"
#include "TripTestBase.h"

/**
 * @brief src/logic/Trip.h と src2/Pipeline.h + TripCompute.h の互換性を検証するテスト
 */
class CompatibilityTest : public TripTestBase {
protected:
  unsigned long   lastGnssTimestamp = 0;
  TripStateDataEx state2;

  void SetUp() override {
    TripTestBase::SetUp();
    lastGnssTimestamp = 0;
    // ... rest of setup
    state2.totalKm        = 0.0f;
    state2.tripDistance   = 0.0f;
    state2.totalMovingMs  = 0;
    state2.maxSpeed       = 0.0f;
    state2.status         = TripStateData::Status::Stopped;
    state2.fixMode        = FixInvalid;
    state2.hasLastCoord   = false;
    state2.lastUpdateTime = 0;
    state2.updateStatus   = UpdateStatus::NoChange;
    state2.currentSpeed   = 0.0f;
    state2.avgSpeed       = 0.0f;
    state2.totalElapsedMs = 0;
    state2.lastLat        = 0.0f;
    state2.lastLon        = 0.0f;
  }

  void updateBoth(unsigned long ms, bool updated = true) {
    // 1. src (Original) を更新
    trip.update(navData, ms, updated);

    // 2. src2 (New Pipeline) を更新
    if (updated) { lastGnssTimestamp = ms; }

    GnssData gnss;
    gnss.navData   = navData;
    gnss.status    = updated ? UpdateStatus::Updated : UpdateStatus::NoChange;
    gnss.timestamp = lastGnssTimestamp;

    Pipeline::computeTrip(state2, gnss, ms);
  }

  void compareStates() {
    auto s1 = trip.getState();

    // 許容誤差 0.001 (浮動小数点演算の順序による微小な差を考慮)
    EXPECT_NEAR(s1.currentSpeed, state2.currentSpeed, 0.001f);
    EXPECT_NEAR(s1.maxSpeed, state2.maxSpeed, 0.001f);
    EXPECT_NEAR(s1.avgSpeed, state2.avgSpeed, 0.01f); // 平均速度は誤差が出やすい
    EXPECT_NEAR(s1.totalKm, state2.totalKm, 0.001f);
    EXPECT_NEAR(s1.tripDistance, state2.tripDistance, 0.001f);
    EXPECT_EQ(s1.totalMovingMs, state2.totalMovingMs);
    EXPECT_EQ(s1.totalElapsedMs, state2.totalElapsedMs);

    // Statusの比較 (Enumの値が一致していることを期待)
    EXPECT_EQ((int)s1.status, (int)state2.status);
  }
};

// --- Test Cases ---

TEST_F(CompatibilityTest, InitialStateMatch) {
  compareStates();
}

TEST_F(CompatibilityTest, MovingSequenceMatch) {
  // 1000ms: 初回更新 (ベースライン設定)
  updateBoth(1000);
  compareStates();

  // 2000ms: 2回目更新 (status -> Moving, hasLastCoord -> true)
  navData.velocity  = 20.0f / 3.6f; // 20 kmh
  navData.latitude  = 35.6812;
  navData.longitude = 139.7671;
  updateBoth(2000);
  compareStates();

  // 3000ms: 3回目更新 (距離加算)
  navData.latitude += 0.001; // 約110m移動
  updateBoth(3000);
  compareStates();

  // 4000ms: 走行継続
  navData.latitude += 0.001;
  updateBoth(4000);
  compareStates();
}

TEST_F(CompatibilityTest, PauseMatch) {
  updateBoth(1000);
  updateBoth(2000);

  // Pause
  trip.pause();
  Pipeline::applyPause(state2);
  EXPECT_EQ(state2.status, TripStateData::Status::Paused);

  updateBoth(3000);
  compareStates();

  // Unpause (Stoppedになる)
  trip.pause();
  state2.status = TripStateData::Status::Stopped;

  updateBoth(4000);
  compareStates();
}

TEST_F(CompatibilityTest, GnssTimeoutMatch) {
  updateBoth(1000);
  updateBoth(2000); // status: Moving

  // 時間だけ経過 (GNSS更新なし)
  updateBoth(3000, false);
  compareStates();

  // タイムアウト発生
  updateBoth(3000 + SIGNAL_TIMEOUT_MS + 100, false);
  compareStates();
}

TEST_F(CompatibilityTest, AverageSpeedEdgeCaseMatch) {
  updateBoth(1000);
  // 非常に短い時間の移動
  updateBoth(1001);
  compareStates();

  // 長時間の停止後の移動
  updateBoth(100000, false);
  navData.latitude += 0.005;
  updateBoth(101000, true);
  compareStates();
}
