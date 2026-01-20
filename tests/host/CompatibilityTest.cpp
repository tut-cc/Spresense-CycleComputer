#include "../../src2/common/DataStructures.h"
#include "TripTestBase.h"

/**
 * @brief src/logic/Trip.h と src2/domain/TripState.h の互換性を検証するテスト
 */
class CompatibilityTest : public TripTestBase {
protected:
  unsigned long lastGnssTimestamp = 0;
  TripState     state2;

  void SetUp() override {
    TripTestBase::SetUp();
    lastGnssTimestamp = 0;
    state2.clearAllData();
  }

  void updateBoth(unsigned long ms, bool updated = true) {
    // 1. src (Original) を更新
    trip.update(navData, ms, updated);

    // 2. src2 (New TripState) を更新
    if (updated) { lastGnssTimestamp = ms; }

    GnssData gnss;
    gnss.navData   = navData;
    gnss.updated   = updated;
    gnss.timestamp = lastGnssTimestamp;

    state2 = TripState(state2, gnss, ms);
  }

  void compareStates() {
    auto s1 = trip.getState();

    // 許容誤差 0.001 (浮動小数点演算の順序による微小な差を考慮)
    EXPECT_NEAR(s1.currentSpeed, state2.speed.current, 0.001f);
    EXPECT_NEAR(s1.maxSpeed, state2.speed.max, 0.001f);

    // 平均速度を更新してから比較
    state2.updateAverageSpeed();
    EXPECT_NEAR(s1.avgSpeed, state2.speed.avg, 0.01f);

    EXPECT_NEAR(s1.totalKm, state2.distance.total, 0.001f);
    EXPECT_NEAR(s1.tripDistance, state2.distance.trip, 0.001f);
    EXPECT_EQ(s1.totalMovingMs, state2.time.moving);
    EXPECT_EQ(s1.totalElapsedMs, state2.time.elapsed);

    // Statusの比較 (Paused以外は一致することを期待)
    if (s1.status != Trip::Status::Paused) { EXPECT_EQ((int)s1.status, (int)state2.status); }
  }
};

// --- Test Cases ---

TEST_F(CompatibilityTest, InitialStateMatch) { compareStates(); }

TEST_F(CompatibilityTest, MovingSequenceMatch) {
  // 1000ms: 初回更新 (ベースライン設定)
  updateBoth(1000);
  compareStates();

  // 2000ms: 2回目更新 (status -> Moving)
  navData.velocity  = 20.0f / 3.6f; // 20 kmh
  navData.latitude  = 35.6812;
  navData.longitude = 139.7671;
  updateBoth(2000);
  compareStates();

  // 3000ms: 3回目更新 (距離加算)
  // 速度20km/hで1秒間 -> 約5.55m
  updateBoth(3000);
  compareStates();

  // 4000ms: 走行継続
  updateBoth(4000);
  compareStates();
}

TEST_F(CompatibilityTest, PauseMatch) {
  updateBoth(1000);
  updateBoth(2000);

  // Pause
  trip.pause();
  state2.status = TripState::Status::Paused;
  EXPECT_EQ(state2.status, TripState::Status::Paused);

  updateBoth(3000);
  compareStates();

  // Unpause (Stoppedになる)
  trip.pause();
  state2.status = TripState::Status::Stopped;

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
  updateBoth(3000 + Config::Gnss::SIGNAL_TIMEOUT_MS + 100, false);
  compareStates();
}

TEST_F(CompatibilityTest, AverageSpeedEdgeCaseMatch) {
  updateBoth(1000);
  // 非常に短い時間の移動
  updateBoth(1001);
  compareStates();

  // 長時間の停止後の移動
  updateBoth(100000, false);
  updateBoth(101000, true);
  compareStates();
}
