#include <Arduino.h>
#include <stddef.h>

#include "Pipeline.h"
#include "hardware/Gnss.h"
#include "logic/DataStore.h"
#include "logic/VoltageMonitor.h"
#include "ui/UI.h"

class App {
private:
  // --- 既存のオブジェクト (ハードウェア制御用) ---
  Gnss           gnss;
  DataStore      dataStore;
  VoltageMonitor voltageMonitor;
  UI             userInterface;

  // --- 新システム (パイプライン) 用データ ---
  Mode::ID        currentMode = Mode::ID::SPD_TIM;
  GnssData        gnssData;
  TripStateDataEx tripData;
  unsigned long   lastSaveMillis = 0;

public:
  App() {}

  void begin() {
    gnss.begin();
    voltageMonitor.begin();
    userInterface.begin();

    // ロード処理
    PersistentData savedData = dataStore.load();
    tripData.totalKm         = savedData.totalDistance;
    tripData.tripDistance    = savedData.tripDistance;
    tripData.totalMovingMs   = savedData.movingTimeMs;
    tripData.maxSpeed        = savedData.maxSpeed;
    tripData.status          = TripStateData::Status::Stopped;
    tripData.fixMode         = FixInvalid;
    tripData.hasLastCoord    = false;
    tripData.lastUpdateTime  = 0;

    lastSaveMillis = millis();
  }

  void update() {
    const unsigned long now = millis();

    // 更新状態をリセット
    tripData.updateStatus = UpdateStatus::NoChange;

    // 1. GNSS入力収集
    gnssData = Pipeline::collectGnss(gnss);

    // 2. ユーザー入力イベント (UI経由で取得)
    Input::Event event = userInterface.getInputEvent();
    if (event != Input::Event::NONE) {
      auto inputResult = Pipeline::handleUserInput<TripStateDataEx>(tripData, currentMode, event);
      tripData         = inputResult.newState;
      currentMode      = inputResult.newMode;

      if (inputResult.shouldClearStorage) {
        dataStore.clear();
        userInterface.showResetMessage(); // 視覚的フィードバック
      }
    }

    // 3. Trip計算 (パイプライン)
    // 時間経過(dt)やGNSS更新を反映 (内部でコピーを最小限にするよう修正済み)
    tripData = Pipeline::computeTrip(tripData, gnssData, now);

    // 4. 永続化処理
    float voltage = voltageMonitor.update();
    if ((now - lastSaveMillis > DataStore::SAVE_INTERVAL_MS) &&
        gnssData.status == UpdateStatus::NoChange) {
      PersistentData pData = Pipeline::createPersistentData(tripData, voltage);
      dataStore.save(pData);
      lastSaveMillis = now;
    }

    // 5. UIの描画 (変化があった場合のみ)
    if (tripData.updateStatus != UpdateStatus::NoChange) {
      DisplayData displayData = Pipeline::createDisplayData(tripData, gnssData, currentMode);
      userInterface.draw(displayData);
    }
  }
};
