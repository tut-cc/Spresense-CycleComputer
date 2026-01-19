#include "TripTestBase.h"

/**
 * @brief 「値が更新されないべき条件」で、実際に値が保持されていることを検証するテスト
 */
class NegativeTest : public TripTestBase {};

// --- 1. 最高速度の非減少性 ---

TEST_F(NegativeTest, MaxSpeed_NeverDecreases) {
  setupMovingState(1000);

  // 30km/h に到達
  navData.velocity = 30.0f / 3.6f;
  updateTrip(2000);
  float recordedMax = trip.getState().maxSpeed;
  EXPECT_NEAR(recordedMax, 30.0f, 0.01f);

  // 10km/h に減速
  navData.velocity = 10.0f / 3.6f;
  updateTrip(3000);

  // 最高速度は 30km/h のまま保持されるべき
  EXPECT_NEAR(trip.getState().maxSpeed, recordedMax, 0.01f);
  EXPECT_NEAR(trip.getState().currentSpeed, 10.0f, 0.01f);
}

// --- 2. 停止中の統計不変性 ---

TEST_F(NegativeTest, NoStatsUpdate_WhenStopped) {
  setupMovingState(1000); // 一度移動状態にする

  // 完全に停止
  navData.velocity = 0.0f;
  updateTrip(2000);
  updateTrip(3000);
  EXPECT_EQ(trip.getState().status, Trip::Status::Stopped);

  unsigned long movingMs = trip.getState().totalMovingMs;
  float         tripDist = trip.getState().tripDistance;

  // 停止したまま時間を進める
  updateTrip(10000);

  // 移動時間も距離も増えていないこと
  EXPECT_EQ(trip.getState().totalMovingMs, movingMs);
  EXPECT_FLOAT_EQ(trip.getState().tripDistance, tripDist);
}

// --- 3. 一時停止中の距離不変性 ---

TEST_F(NegativeTest, NoTripDistanceUpdate_WhenPaused) {
  setupMovingState(1000);

  // 一時停止に切り替え
  trip.pause();
  EXPECT_EQ(trip.getState().status, Trip::Status::Paused);

  float initialTripDist = trip.getState().tripDistance;
  float initialTotalKm  = trip.getState().totalKm;

  // 移動しながら更新（速度も位置も有効）
  navData.velocity = 20.0f / 3.6f;
  navData.moveByMeters(100.0f);
  updateTrip(2000);

  // TripDistance (その回の走行距離) は増えてはいけない
  EXPECT_FLOAT_EQ(trip.getState().tripDistance, initialTripDist);

  // 仕様確認: totalKm (オドメーター) は Pause 中も増えるべきか？
  // 現状の Trip.h 141行目: if (state.status != Status::Paused) { state.tripDistance += deltaKm; }
  // 169行目: state.totalKm += delta; (statusに関わらず加算)
  // つまり、totalKm は増えるのが正解。
  EXPECT_GT(trip.getState().totalKm, initialTotalKm);
}

// --- 4. 無効なFix状態での距離不変性 ---

TEST_F(NegativeTest, NoDistanceUpdate_OnInvalidFix) {
  setupMovingState(1000);

  float initialTotalKm = trip.getState().totalKm;

  // Fixを失う
  navData.posFixMode = FixInvalid;
  // 大幅に座標を動かす
  navData.moveByMeters(500.0f);
  updateTrip(2000);

  // Fixがない場合は距離は一切増えてはいけない
  EXPECT_FLOAT_EQ(trip.getState().currentSpeed, 0.0f);
  EXPECT_FLOAT_EQ(trip.getState().totalKm, initialTotalKm);
}

// --- 5. 移動フラグ(moving)が偽の時の距離不変性 ---

TEST_F(NegativeTest, NoDistanceUpdate_WhenVelocityTooLow) {
  setupMovingState(1000);

  float initialTotalKm = trip.getState().totalKm;

  // 座標は動いているが、速度データが閾値(0.001km/h)以下の場合
  // （GPSの微小な座標ふらつきを想定）
  navData.velocity = 0.00001f / 3.6f;
  navData.moveByMeters(10.0f);
  updateTrip(2000);

  // 速度が低すぎるため、移動とはみなされず距離も増えない
  EXPECT_EQ(trip.getState().status, Trip::Status::Stopped);
  EXPECT_FLOAT_EQ(trip.getState().totalKm, initialTotalKm);
}

// --- 6. 未初期化状態での更新拒否 ---

TEST_F(NegativeTest, NoUpdate_BeforeFirstFix) {
  Trip newTrip; // begin() 直後の状態
  newTrip.begin();

  // 座標を動かしても、最初の1回目は lastCoord のセットのみに使われる
  navData.moveByMeters(100.0f);
  newTrip.update(navData, 1000, true); // 1回目 (hasLastUpdate set)

  EXPECT_FLOAT_EQ(newTrip.getState().totalKm, 0.0f);

  navData.moveByMeters(100.0f);
  newTrip.update(navData, 2000, true); // 2回目 (hasLastCoord set)

  // まだ距離加算は始まらない（基準点が決まっただけ）
  EXPECT_FLOAT_EQ(newTrip.getState().totalKm, 0.0f);

  navData.moveByMeters(100.0f);
  newTrip.update(navData, 3000, true); // 3回目でようやく加算
  EXPECT_GT(newTrip.getState().totalKm, 0.0f);
}
