#include <Arduino.h>
#include <stddef.h>

#include "Pipeline.h"
#include "TripCompute.h"
#include "hardware/Gnss.h"
#include "logic/DataStore.h"
#include "logic/VoltageMonitor.h"
#include "ui/UI.h"

class App {
private:
  // --- Hardware Abstractions ---
  Gnss           gnss;
  DataStore      dataStore;
  VoltageMonitor voltageMonitor;
  UI             userInterface;

  // --- State ---
  Mode::ID        currentMode = Mode::ID::SPD_TIM;
  GnssData        gnssData;
  TripStateDataEx tripState[2]; // Double buffer (0: Prev, 1: Curr)
  int             currentIdx     = 0;
  unsigned long   lastSaveMs     = 0;
  unsigned long   lastUiUpdateMs = 0;

public:
  App() = default;

  void begin() {
    gnss.begin();
    voltageMonitor.begin();
    userInterface.begin();

    // Init state from persistence
    PersistentData saved = dataStore.load();
    for (auto &state : tripState) {
      state.resetAll();
      state.totalKm       = saved.totalDistance;
      state.tripDistance  = saved.tripDistance;
      state.totalMovingMs = saved.movingTimeMs;
      state.maxSpeed      = saved.maxSpeed;
    }

    lastSaveMs = millis();
  }

  void update() {
    const unsigned long now     = millis();
    const int           prevIdx = currentIdx;
    const int           currIdx = 1 - currentIdx;

    // 1. Prepare current buffer by copying from previous
    tripState[currIdx] = tripState[prevIdx];
    tripState[currIdx].resetMeta();

    // 2. Capture Inputs
    gnssData           = Pipeline::collectGnss(gnss);
    Input::Event event = userInterface.getInputEvent();

    // 3. Process User Input
    if (event != Input::Event::NONE) {
      auto result = Pipeline::handleUserInput(tripState[currIdx], currentMode, event);
      currentMode = result.newMode;

      if (result.shouldClearStorage) {
        dataStore.clear();
        userInterface.showResetMessage();
      }
    }

    // 4. Compute Trip Logic
    Pipeline::computeTrip(tripState[currIdx], gnssData, now);

    // 5. Persistence
    handlePersistence(tripState[currIdx], now);

    // 6. UI Update
    handleUI(tripState[prevIdx], tripState[currIdx], now);

    // 7. Swap Buffers
    currentIdx = currIdx;
  }

private:
  void handlePersistence(const TripStateDataEx &state, unsigned long now) {
    if (now - lastSaveMs < DataStore::SAVE_INTERVAL_MS) return;

    // Only save when GNSS is stable or not updating to avoid IO jitter
    if (gnssData.status == UpdateStatus::NoChange) {
      float          v     = voltageMonitor.update();
      PersistentData pData = Pipeline::createPersistentData(state, v);
      dataStore.save(pData);
      lastSaveMs = now;
    }
  }

  void handleUI(const TripStateDataEx &prev, const TripStateDataEx &curr, unsigned long now) {
    bool periodic = (now - lastUiUpdateMs >= 500);
    bool changed  = Pipeline::isChanged(prev, curr);
    bool forced   = (curr.updateStatus == UpdateStatus::ForceUpdate);
    bool gnssUpd  = (gnssData.status == UpdateStatus::Updated);

    if (changed || forced || gnssUpd || periodic) {
      DisplayData dData = Pipeline::createDisplayData(curr, gnssData, currentMode);
      userInterface.draw(dData);
      lastUiUpdateMs = now;
    }
  }
};
