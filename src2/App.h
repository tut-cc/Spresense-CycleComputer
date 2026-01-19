#include <Arduino.h>
#include <stddef.h>

#include "hardware/Clock.h"
#include "hardware/Gnss.h"
#include "logic/DataStore.h"
#include "logic/Pipeline.h"
#include "logic/TripCompute.h"
#include "logic/VoltageMonitor.h"
#include "ui/UI.h"

namespace Formatter {

inline void formatSpeed(float speedKmh, char *buffer, size_t size) {
  snprintf(buffer, size, "%4.1f", speedKmh);
}

inline void formatDistance(float distanceKm, char *buffer, size_t size) {
  snprintf(buffer, size, "%5.2f", distanceKm);
}

inline void formatDuration(unsigned long millis, char *buffer, size_t size) {
  const unsigned long seconds = millis / 1000;
  const unsigned long h       = seconds / 3600;
  const unsigned long m       = (seconds % 3600) / 60;
  const unsigned long s       = seconds % 60;

  if (h > 0) {
    snprintf(buffer, size, "%lu:%02lu:%02lu", h, m, s);
    return;
  }

  snprintf(buffer, size, "%02lu:%02lu", m, s);
}

} // namespace Formatter

class App {
private:
  Gnss           gnss;
  Clock          systemClock;
  DataStore      dataStore;
  VoltageMonitor voltageMonitor;
  UI             userInterface;

  Mode            currentMode = Mode::SPD_TIM;
  GnssData        gnssData;
  TripStateDataEx tripState[2];
  DisplayFrame    frames[2];
  SaveData        saveBuffers[2];
  int             currentIdx     = 0;
  int             frameIdx       = 0;
  int             saveIdx        = 0;
  unsigned long   lastSaveMs     = 0;
  unsigned long   lastUiUpdateMs = 0;

public:
  App() = default;

  void begin() {
    gnss.begin();
    systemClock.begin();
    voltageMonitor.begin();
    userInterface.begin();

    SaveData saved = dataStore.load();
    for (auto &state : tripState) {
      state.resetAll();
      state.totalKm       = saved.totalDistance;
      state.tripDistance  = saved.tripDistance;
      state.totalMovingMs = saved.movingTimeMs;
      state.maxSpeed      = saved.maxSpeed;
    }

    saveBuffers[0] = saved;
    saveBuffers[1] = saved;

    lastSaveMs = millis();
  }

  void update() {
    const unsigned long now     = millis();
    const int           prevIdx = currentIdx;
    const int           currIdx = 1 - currentIdx;

    tripState[currIdx] = tripState[prevIdx];
    tripState[currIdx].resetMeta();

    gnssData           = Pipeline::collectGnss(gnss);
    Input::Event event = userInterface.getInputEvent();

    if (gnssData.status == UpdateStatus::Updated &&
        (SpFixMode)gnssData.navData.posFixMode != FixInvalid) {
      systemClock.sync(gnssData.navData.time);
    }

    if (event != Input::Event::NONE) {
      auto result = Pipeline::handleUserInput(tripState[currIdx], currentMode, event);
      currentMode = result.newMode;

      if (result.shouldClearStorage) {
        dataStore.clear();
        userInterface.showResetMessage();
        frames[0] = DisplayFrame();
        frames[1] = DisplayFrame();

        TripStateDataEx emptyState;
        emptyState.resetAll();
        // voltage is not reset, but here we can use 0 or current
        SaveData emptySave = Pipeline::createSaveData(emptyState, 0.0f);
        saveBuffers[0]     = emptySave;
        saveBuffers[1]     = emptySave;
      }
    }

    Pipeline::computeTrip(tripState[currIdx], gnssData, now);
    handleSave(tripState[currIdx], now);
    handleUI(tripState[prevIdx], tripState[currIdx], now);
    currentIdx = currIdx;
  }

private:
  void handleSave(const TripStateDataEx &state, unsigned long now) {
    if (now - lastSaveMs < DataStore::SAVE_INTERVAL_MS) return;
    if (gnssData.status != UpdateStatus::NoChange) return;

    float    v     = voltageMonitor.update();
    SaveData pData = Pipeline::createSaveData(state, v);

    const int prevSaveIdx = saveIdx;
    saveIdx               = 1 - saveIdx;
    saveBuffers[saveIdx]  = pData;

    if (saveBuffers[saveIdx] != saveBuffers[prevSaveIdx]) { dataStore.save(saveBuffers[saveIdx]); }
    lastSaveMs = now;
  }

  void handleUI(const TripStateDataEx &prev, const TripStateDataEx &curr, unsigned long now) {
    bool periodic = (now - lastUiUpdateMs >= 500);
    bool changed  = Pipeline::isChanged(prev, curr);
    bool forced   = (curr.updateStatus == UpdateStatus::ForceUpdate);
    bool gnssUpd  = (gnssData.status == UpdateStatus::Updated);

    if (changed || forced || gnssUpd || periodic) {
      SpGnssTime  currentTime = systemClock.now();
      DisplayData dData = Pipeline::createDisplayData(curr, gnssData, currentTime, currentMode);

      const int prevFrameIdx = frameIdx;
      frameIdx               = 1 - frameIdx;
      frames[frameIdx]       = createFrame(dData);

      if (frames[frameIdx] != frames[prevFrameIdx]) {
        userInterface.draw(frames[frameIdx]);
        lastUiUpdateMs = now;
      }
    }
  }

  DisplayFrame createFrame(const DisplayData &data) const {
    DisplayFrame frame;

    switch (data.fixMode) {
    case Fix2D:
      strcpy(frame.header.fixStatus, "2D");
      break;
    case Fix3D:
      strcpy(frame.header.fixStatus, "3D");
      break;
    default:
      strcpy(frame.header.fixStatus, "WAIT");
      break;
    }

    if (data.modeSpeedLabel) strcpy(frame.header.modeSpeed, data.modeSpeedLabel);
    if (data.modeTimeLabel) strcpy(frame.header.modeTime, data.modeTimeLabel);

    Formatter::formatSpeed(data.mainValue, frame.main.value, sizeof(frame.main.value));
    if (data.mainUnit) strcpy(frame.main.unit, data.mainUnit);

    if (data.shouldBlink) {
      strcpy(frame.sub.value, "");
      strcpy(frame.sub.unit, "");
    } else {
      switch (data.subType) {
      case DisplayData::SubType::Duration:
        Formatter::formatDuration(data.subValue.durationMs, frame.sub.value,
                                  sizeof(frame.sub.value));
        break;
      case DisplayData::SubType::Distance:
        Formatter::formatDistance(data.subValue.distanceKm, frame.sub.value,
                                  sizeof(frame.sub.value));
        break;
      case DisplayData::SubType::Clock:
        snprintf(frame.sub.value, sizeof(frame.sub.value), "%02d:%02d",
                 data.subValue.clockTime.hour, data.subValue.clockTime.minute);
        break;
      }
      if (data.subUnit) strcpy(frame.sub.unit, data.subUnit);
    }
    return frame;
  }
};
