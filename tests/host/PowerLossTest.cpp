#include "App.h"
#include "TripTestBase.h"

/**
 * @brief 発電機給電による頻繁な電源ロスを想定したテストスイート
 *
 * サイクルコンピュータは発電機から給電されるため、以下のシナリオが頻繁に発生する:
 * - 停車時や低速時の発電不足による電源断
 * - 坂道での速度低下による瞬断
 * - 走行中の複数回の電源ロス・復旧サイクル
 */
class PowerLossTest : public TripTestBase {
protected:
  DataStore dataStore;

  // 電源断をシミュレート（システム再起動）
  void simulatePowerLoss(Trip &trip, DataStore &ds) {
    // 現在の状態を保存
    AppData            currentData;
    const Trip::State &state  = trip.getState();
    currentData.totalDistance = state.totalKm;
    currentData.tripDistance  = state.tripDistance;
    currentData.movingTimeMs  = state.totalMovingMs;
    currentData.maxSpeed      = state.maxSpeed;
    currentData.voltage       = 4.0f;
    ds.save(currentData);
  }

  // 電源復旧をシミュレート（新しいTripインスタンスでデータ復元）
  Trip simulatePowerRecovery(DataStore &ds) {
    Trip newTrip;
    newTrip.begin();
    AppData loaded = ds.load();
    newTrip.restore(loaded.totalDistance, loaded.tripDistance, loaded.movingTimeMs,
                    loaded.maxSpeed);
    return newTrip;
  }

  void setupMovingStateForTrip(Trip &t, unsigned long startMillis) {
    navData.posFixMode = Fix3D;
    navData.velocity   = 10.0f / 3.6f;
    navData.latitude   = 35.6812;
    navData.longitude  = 139.7671;

    t.update(navData, startMillis, true);
    t.update(navData, startMillis + 100, true);
  }
};

// --- 1. 頻繁な電源断・復旧サイクル ---

TEST_F(PowerLossTest, FrequentPowerCycles) {
  // 走行開始
  setupMovingState(1000);
  navData.moveByMeters(500.0f);
  updateTrip(5000);

  float dist1 = trip.getState().totalKm;
  EXPECT_GT(dist1, 0.0f);

  // 1回目の電源断・復旧
  simulatePowerLoss(trip, dataStore);
  Trip trip2 = simulatePowerRecovery(dataStore);
  EXPECT_FLOAT_EQ(trip2.getState().totalKm, dist1);

  // 走行継続（新しいTripインスタンスなので座標を再設定）
  setupMovingStateForTrip(trip2, 6000);
  navData.moveByMeters(300.0f);
  trip2.update(navData, 8000, true);
  float dist2 = trip2.getState().totalKm;
  EXPECT_GT(dist2, dist1);

  // 2回目の電源断・復旧
  simulatePowerLoss(trip2, dataStore);
  Trip trip3 = simulatePowerRecovery(dataStore);
  EXPECT_FLOAT_EQ(trip3.getState().totalKm, dist2);

  // 3回目の電源断・復旧
  simulatePowerLoss(trip3, dataStore);
  Trip trip4 = simulatePowerRecovery(dataStore);
  EXPECT_FLOAT_EQ(trip4.getState().totalKm, dist2);
}

TEST_F(PowerLossTest, PowerLossDuringMovement) {
  // 走行中に電源断
  setupMovingState(1000);
  navData.moveByMeters(1000.0f);
  updateTrip(10000);

  EXPECT_EQ(trip.getState().status, Trip::Status::Moving);
  float totalKm = trip.getState().totalKm;

  // 電源断・復旧
  simulatePowerLoss(trip, dataStore);
  Trip newTrip = simulatePowerRecovery(dataStore);

  // 距離は保持されているが、ステータスはStoppedにリセット
  EXPECT_FLOAT_EQ(newTrip.getState().totalKm, totalKm);
  EXPECT_EQ(newTrip.getState().status, Trip::Status::Stopped);
  EXPECT_FLOAT_EQ(newTrip.getState().currentSpeed, 0.0f);
}

// --- 2. データ整合性の保証 ---

TEST_F(PowerLossTest, CorruptedDataRecovery) {
  // 正常なデータを保存
  setupMovingState(1000);
  navData.moveByMeters(500.0f);
  updateTrip(5000);
  simulatePowerLoss(trip, dataStore);

  // EEPROMを破損させる（CRC破損）
  EEPROM.buffer[28] ^= 0xFF;

  // 復旧時はデフォルト値が返される
  Trip newTrip = simulatePowerRecovery(dataStore);
  EXPECT_FLOAT_EQ(newTrip.getState().totalKm, 0.0f);
  EXPECT_FLOAT_EQ(newTrip.getState().tripDistance, 0.0f);
}

TEST_F(PowerLossTest, MagicNumberValidation) {
  // データを保存
  AppData data = {100.5f, 10.2f, 3600000, 25.5f, 4.2f};
  dataStore.save(data);

  // 正常に保存された後は有効なマジックナンバーが設定されている
  uint32_t magic;
  std::memcpy(&magic, &EEPROM.buffer[0], 4);
  EXPECT_EQ(magic, MAGIC_NUMBER);

  // データが正しく読み込める
  AppData loaded = dataStore.load();
  EXPECT_FLOAT_EQ(loaded.totalDistance, 100.5f);
}

// --- 3. 実走行シナリオ ---

TEST_F(PowerLossTest, LongRideWithMultiplePowerLosses) {
  // 100km の長距離走行をシミュレート（複数回の電源断を含む）
  Trip  currentTrip     = trip;
  float expectedTotalKm = 0.0f;

  for (int segment = 0; segment < 10; ++segment) {
    // 各セグメントで10km走行
    setupMovingStateForTrip(currentTrip, 1000 + segment * 100000);

    for (int i = 0; i < 100; ++i) {
      navData.moveByMeters(100.0f); // 100m移動
      currentTrip.update(navData, 2000 + segment * 100000 + i * 1000, true);
    }

    expectedTotalKm += 10.0f; // 10km追加

    // セグメント終了時に電源断・復旧
    simulatePowerLoss(currentTrip, dataStore);
    currentTrip = simulatePowerRecovery(dataStore);

    // 距離が累積されていることを確認（誤差許容）
    EXPECT_NEAR(currentTrip.getState().totalKm, expectedTotalKm, 1.0f);
  }

  // 最終的に約100km走行していることを確認
  EXPECT_NEAR(currentTrip.getState().totalKm, 100.0f, 5.0f);
}

TEST_F(PowerLossTest, StopAndGoWithPowerLoss) {
  // 信号待ちや休憩を含む市街地走行
  setupMovingState(1000);

  // 走行
  navData.moveByMeters(500.0f);
  updateTrip(10000);
  float dist1 = trip.getState().totalKm;

  // 停止（信号待ち）
  navData.velocity = 0.0f;
  updateTrip(20000);
  EXPECT_EQ(trip.getState().status, Trip::Status::Stopped);

  // 停止中に電源断
  simulatePowerLoss(trip, dataStore);
  Trip trip2 = simulatePowerRecovery(dataStore);

  // 再発進
  navData.velocity = 20.0f / 3.6f;
  trip2.update(navData, 30000, true);
  trip2.update(navData, 31000, true);
  EXPECT_EQ(trip2.getState().status, Trip::Status::Moving);

  // さらに走行
  navData.moveByMeters(500.0f);
  trip2.update(navData, 40000, true);
  EXPECT_GT(trip2.getState().totalKm, dist1);
}

TEST_F(PowerLossTest, TunnelGnssLossWithPowerLoss) {
  // トンネル内でのGNSS信号ロスと電源断の複合
  setupMovingState(1000);
  navData.moveByMeters(500.0f);
  updateTrip(5000);

  float distBeforeTunnel = trip.getState().totalKm;

  // トンネル進入（GNSS信号ロス）
  updateTrip(5000 + SIGNAL_TIMEOUT_MS + 100, false);
  EXPECT_EQ(trip.getState().status, Trip::Status::Stopped);

  // トンネル内で電源断
  simulatePowerLoss(trip, dataStore);
  Trip trip2 = simulatePowerRecovery(dataStore);

  // 距離は保持されている
  EXPECT_FLOAT_EQ(trip2.getState().totalKm, distBeforeTunnel);

  // トンネル脱出（GNSS復旧）
  trip2.update(navData, 20000, true);
  trip2.update(navData, 21000, true);
  EXPECT_EQ(trip2.getState().status, Trip::Status::Moving);
}

// --- 4. 最大速度の保持 ---

TEST_F(PowerLossTest, MaxSpeedRetention) {
  setupMovingState(1000);

  // 高速で走行
  navData.velocity = 40.0f / 3.6f; // 40km/h
  updateTrip(2000);
  EXPECT_NEAR(trip.getState().maxSpeed, 40.0f, 0.1f);

  // 電源断・復旧
  simulatePowerLoss(trip, dataStore);
  Trip trip2 = simulatePowerRecovery(dataStore);

  // 最高速度が保持されている
  EXPECT_NEAR(trip2.getState().maxSpeed, 40.0f, 0.1f);

  // 低速で走行
  navData.velocity = 15.0f / 3.6f; // 15km/h
  trip2.update(navData, 3000, true);
  trip2.update(navData, 4000, true);

  // 最高速度は更新されない
  EXPECT_NEAR(trip2.getState().maxSpeed, 40.0f, 0.1f);
}

// --- 5. 移動時間の累積 ---

TEST_F(PowerLossTest, MovingTimeAccumulation) {
  setupMovingState(1000);
  updateTrip(2000); // +1000ms

  unsigned long time1 = trip.getState().totalMovingMs;
  // setupMovingStateは100ms消費するので、実際は900ms
  EXPECT_EQ(time1, 900);

  // 電源断・復旧
  simulatePowerLoss(trip, dataStore);
  Trip trip2 = simulatePowerRecovery(dataStore);

  // 移動時間が保持されている
  EXPECT_EQ(trip2.getState().totalMovingMs, time1);

  // 走行継続（新しいTripインスタンスなので座標を再設定）
  setupMovingStateForTrip(trip2, 3000); // これも100ms消費
  trip2.update(navData, 5000, true);    // +2000ms

  // 移動時間が累積されている（900 + 100 + 1800 = 2800）
  EXPECT_EQ(trip2.getState().totalMovingMs, 2800);
}

// --- 6. EEPROMの書き込み寿命を考慮 ---

TEST_F(PowerLossTest, MinimizeEepromWrites) {
  // 同じデータの繰り返し保存は書き込みを発生させない
  AppData data = {100.5f, 10.2f, 3600000, 25.5f, 4.2f};

  dataStore.save(data);
  uint32_t writeCount1 = EEPROM.writeCount;

  // 100回同じデータを保存
  for (int i = 0; i < 100; ++i) { dataStore.save(data); }

  // 書き込み回数が増えていないことを確認
  EXPECT_EQ(EEPROM.writeCount, writeCount1);
}

TEST_F(PowerLossTest, VoltageChangeDoesNotTriggerSave) {
  // voltage以外が同じ場合、保存されない（EEPROM寿命を延ばす）
  AppData data = {100.5f, 10.2f, 3600000, 25.5f, 4.2f};
  dataStore.save(data);

  EEPROM.clearWriteCount();

  // voltageのみ変更して100回保存
  for (int i = 0; i < 100; ++i) {
    data.voltage = 3.5f + i * 0.01f;
    dataStore.save(data);
  }

  // 書き込みが発生していないことを確認
  EXPECT_EQ(EEPROM.writeCount, 0);
}
