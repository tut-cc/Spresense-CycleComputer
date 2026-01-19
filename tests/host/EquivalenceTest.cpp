#include "TripTestBase.h"

/**
 * @brief 同値分割法（Equivalence Partitioning）に基づくテスト
 */
class EquivalenceTest : public TripTestBase {};

// --- 1. GNSS測位状態 (Fix Mode) ---

TEST_F(EquivalenceTest, FixMode_Valid_3D) {
  navData.posFixMode = Fix3D;
  setupMovingState();
  EXPECT_EQ(trip.getState().status, Trip::Status::Moving);
}

TEST_F(EquivalenceTest, FixMode_Valid_2D) {
  navData.posFixMode = Fix2D;
  setupMovingState();
  EXPECT_EQ(trip.getState().status, Trip::Status::Moving);
}

TEST_F(EquivalenceTest, FixMode_Invalid_Invalid) {
  navData.posFixMode = FixInvalid;
  setupMovingState();
  // 無効なFix状態では、速度があってもStoppedになるべき
  EXPECT_EQ(trip.getState().status, Trip::Status::Stopped);
}

// --- 2. 走行速度 (Speed) ---

TEST_F(EquivalenceTest, Speed_Moving_Typical) {
  navData.velocity = 20.0f / 3.6f;
  setupMovingState();
  EXPECT_EQ(trip.getState().status, Trip::Status::Moving);
  EXPECT_NEAR(trip.getState().currentSpeed, 20.0f, 0.01f);
}

TEST_F(EquivalenceTest, Speed_Stopped_Zero) {
  navData.velocity = 0.0f;
  setupMovingState();
  EXPECT_EQ(trip.getState().status, Trip::Status::Stopped);
}

TEST_F(EquivalenceTest, Speed_Stopped_VerySlow) {
  navData.velocity = (MIN_MOVING_SPEED_KMH - 0.0001f) / 3.6f;
  setupMovingState();
  EXPECT_EQ(trip.getState().status, Trip::Status::Stopped);
}

// --- 3. 座標変化 (Distance Delta) ---

TEST_F(EquivalenceTest, DistanceDelta_Valid) {
  setupMovingState(1000);

  navData.moveByMeters(10.0f); // 10m移動
  updateTrip(2000);

  EXPECT_GT(trip.getState().totalKm, 0.0f);
  EXPECT_LT(trip.getState().totalKm, 0.1f);
}

TEST_F(EquivalenceTest, DistanceDelta_Noise_TooSmall) {
  setupMovingState(1000);

  navData.moveByMeters(0.1f); // 0.1m (MIN_DELTA 2m 以下)
  updateTrip(2000);

  EXPECT_FLOAT_EQ(trip.getState().totalKm, 0.0f);
}

TEST_F(EquivalenceTest, DistanceDelta_Jump_TooLarge) {
  setupMovingState(1000);

  navData.moveByMeters(2000.0f); // 2km (MAX_DELTA 1km 以上)
  updateTrip(2000);

  EXPECT_FLOAT_EQ(trip.getState().totalKm, 0.0f);
}

// --- 4. 座標の妥当性 (Coordinate Validity) ---

TEST_F(EquivalenceTest, Coordinate_Invalid_Zero) {
  navData.latitude  = 0.0f;
  navData.longitude = 0.0f;
  setupMovingState();

  navData.moveByMeters(10.0f);
  updateTrip(3000);
  EXPECT_FLOAT_EQ(trip.getState().totalKm, 0.0f);
}

// --- 5. 追加の重要ケース (元LogicTestより) ---

TEST_F(EquivalenceTest, GnssTimeout) {
  setupMovingState(1000);
  EXPECT_EQ(trip.getState().status, Trip::Status::Moving);

  // Timeout (isGnssUpdated = false)
  // setupMovingState(1000) で lastGnssUpdateMs は 1100 になっている
  updateTrip(1100 + SIGNAL_TIMEOUT_MS + 100, false);
  EXPECT_EQ(trip.getState().status, Trip::Status::Stopped);
  EXPECT_FLOAT_EQ(trip.getState().currentSpeed, 0.0f);
}

TEST_F(EquivalenceTest, PauseToggle) {
  trip.pause();
  EXPECT_EQ(trip.getState().status, Trip::Status::Paused);
  trip.pause();
  EXPECT_EQ(trip.getState().status, Trip::Status::Stopped);
}
