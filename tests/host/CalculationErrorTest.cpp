#include "TripTestBase.h"
#include <limits>

/**
 * @brief 内部計算におけるエラー（精度低下、溢れ、ゼロ除算など）の可能性を検証するテスト
 */
class CalculationTest : public TripTestBase {};

// --- 1. 統計計算の安定性 ---

TEST_F(CalculationTest, AverageSpeed_ZeroDivision) {
  EXPECT_FLOAT_EQ(trip.getState().avgSpeed, 0.0f);

  navData.velocity = 0.0f;
  updateTrip(1000);
  updateTrip(2000);
  EXPECT_FLOAT_EQ(trip.getState().avgSpeed, 0.0f);
  EXPECT_FALSE(std::isnan(trip.getState().avgSpeed));
}

TEST_F(CalculationTest, AverageSpeed_SmallTime) {
  setupMovingState(1000);

  // 実際に少し移動させる
  navData.moveByMeters(10.0f);
  updateTrip(1101); // +1ms 経過

  EXPECT_FALSE(std::isnan(trip.getState().avgSpeed));
  EXPECT_GT(trip.getState().avgSpeed, 0.0f);
}

// --- 2. 距離計算の精度と累積誤差 ---

TEST_F(CalculationTest, Odometer_PrecisionLoss) {
  trip.restore(10000.0f, 0.0f, 0, 0.0f);
  setupMovingState(1000);

  float initialTotal = trip.getState().totalKm;

  // 10mの移動を100回繰り返す
  for (int i = 0; i < 100; ++i) {
    navData.moveByMeters(11.0f);
    updateTrip(2000 + i * 1000);
  }

  EXPECT_NEAR(trip.getState().totalKm, initialTotal + 1.1f, 0.1f);
}

// --- 3. 座標計算の境界ケース ---

TEST_F(CalculationTest, Distance_NearPoles) {
  navData.latitude = 89.9f;
  setupMovingState(1000);

  float startKm = trip.getState().totalKm;
  navData.moveByMeters(10.0f);
  updateTrip(2000);

  EXPECT_GT(trip.getState().totalKm, startKm);
}

TEST_F(CalculationTest, Distance_LongitudeWrap) {
  navData.longitude = 179.999f;
  setupMovingState(1000);

  float startKm     = trip.getState().totalKm;
  navData.longitude = -179.999f; // 反対側へジャンプ
  updateTrip(2000);

  // 跳躍は無視されるべき
  EXPECT_FLOAT_EQ(trip.getState().totalKm, startKm);
}

// --- 4. 時間のオーバーフロー ---

TEST_F(CalculationTest, Time_OverflowHandling) {
  unsigned long nearlyMax = std::numeric_limits<unsigned long>::max() - 500;

  setupMovingState(nearlyMax);

  updateTrip(nearlyMax + 1000); // Wrap-around

  // 2回目のupdate(nearlyMax+100)から3回目のupdate(nearlyMax+1000)の差分 900ms が加算される
  EXPECT_EQ(trip.getState().totalMovingMs, 900);
}

// --- 5. 無効な数値入力 ---

TEST_F(CalculationTest, Speed_NaN_Input) {
  setupMovingState(1000);
  navData.velocity = std::numeric_limits<float>::quiet_NaN();
  updateTrip(2000);

  EXPECT_FALSE(std::isnan(trip.getState().currentSpeed));
  EXPECT_FLOAT_EQ(trip.getState().currentSpeed, 0.0f);
}
