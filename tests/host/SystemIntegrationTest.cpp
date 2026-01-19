#include "App.h"
#include "TripTestBase.h"

/**
 * @brief システム全体の統合的な挙動（保存、同期、寿命など）に関するテスト
 */
class SystemIntegrationTest : public TripTestBase {
protected:
  DataStore dataStore;
  // Trip は TripTestBase にあるのでそれを使用
};

// --- 1. GNSSアクティブ時の保存ブロック ---

TEST_F(SystemIntegrationTest, Persistence_SaveBlockedByActiveGnss) {
  DataPersistence dp(dataStore, trip);
  _mock_millis = 0;
  dp.load();
  EEPROM.clearWriteCount();

  // Tripのデータを変更して、前回の保存内容と異なるようにする
  navData.moveByMeters(100.0f);
  updateTrip(100); // hasLastUpdate = true
  updateTrip(200); // hasLastCoord = true, status -> Moving
  navData.moveByMeters(100.0f);
  updateTrip(300); // totalKm updated

  // 保存インターバルを超える時間を経過させる
  _mock_millis = DataStore::SAVE_INTERVAL_MS + 1000;

  // GNSSが更新されている間は、保存がスキップされるべき
  dp.update(true, 4.0f);
  EXPECT_EQ(EEPROM.writeCount, 0);

  // GNSSが停止した瞬間に保存が実行されるべき
  dp.update(false, 4.0f);
  EXPECT_GT(EEPROM.writeCount, 0);
}

// --- 2. EEPROMの無駄な書き込み抑制 ---

TEST_F(SystemIntegrationTest, DataStore_RedundantWriteSuppression) {
  AppData data = {10.5f, 1.2f, 3600, 25.0f, 4.2f};

  // 初回保存
  EEPROM.clearWriteCount();
  dataStore.save(data);
  uint32_t firstWriteCount = EEPROM.writeCount;
  EXPECT_GT(firstWriteCount, 0);

  // 同じデータを再度保存しようとする
  dataStore.save(data);

  // 書き込み回数が増えていないことを確認（早期リターンしている）
  EXPECT_EQ(EEPROM.writeCount, firstWriteCount);

  // 一部でもデータが変われば書き込まれる
  data.tripDistance += 0.01f;
  dataStore.save(data);
  EXPECT_GT(EEPROM.writeCount, firstWriteCount);
}

// --- 3. 電源喪失を想定した復旧サイクル ---

TEST_F(SystemIntegrationTest, FullCycle_PowerLossRecovery) {
  // 1. 走行してデータを蓄積
  setupMovingState(1000);
  navData.moveByMeters(500.0f);
  updateTrip(5000);

  const float currentTripDist = trip.getState().tripDistance;
  const float currentTotalKm  = trip.getState().totalKm;

  // 2. 手動で保存（またはPersistence経由）
  AppData dataToSave;
  dataToSave.totalDistance = trip.getState().totalKm;
  dataToSave.tripDistance  = trip.getState().tripDistance;
  dataToSave.movingTimeMs  = trip.getState().totalMovingMs;
  dataToSave.maxSpeed      = trip.getState().maxSpeed;
  dataStore.save(dataToSave);

  // 3. システムリセット（App/Tripの再生成を模倣）
  Trip newTrip;
  newTrip.begin();
  DataPersistence newDp(dataStore, newTrip);

  // 4. ロード
  newDp.load();

  // 5. 状態が復元されているか
  EXPECT_FLOAT_EQ(newTrip.getState().tripDistance, currentTripDist);
  EXPECT_FLOAT_EQ(newTrip.getState().totalKm, currentTotalKm);
  EXPECT_EQ(newTrip.getState().status, Trip::Status::Stopped);
}

// --- 4. 長時間ブロック後のリカバリ ---

TEST_F(SystemIntegrationTest, LongLoopBlock_Accuracy) {
  setupMovingState(1000);

  // ループが10秒間止まったとする
  _mock_millis = 11000;
  // その間の最後のGNSSデータは100m先を示していたとする
  navData.moveByMeters(100.0f);

  updateTrip(_mock_millis); // dt = 11000 - 1100 = 9900

  // dt が 9900ms として計算され、移動時間に正しく加算されるべき
  EXPECT_EQ(trip.getState().totalMovingMs, 9900);
  // 現時点のロジックでは currentSpeed は GNSS の速度データをそのまま反映する
  EXPECT_NEAR(trip.getState().currentSpeed, 10.0f, 0.1f);
}
