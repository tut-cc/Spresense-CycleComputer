#pragma once

#include <Arduino.h>
#include <stddef.h>

#include "common/Formatter.h"
#include "domain/DataStore.h"
#include "domain/MvuPipeline.h"
#include "domain/PowerManager.h"
#include "domain/TripCompute.h"
#include "hardware/Clock.h"
#include "hardware/Gnss.h"
#include "ui/UI.h"

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

    if (saveBuffers[saveIdx] != saveBuffers[prevSaveIdx]) dataStore.save(saveBuffers[saveIdx]);
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

  using FormatterFunc = void (*)(const DisplayData &, char *, size_t);

  static void fmtDuration(const DisplayData &d, char *b, size_t s) {
    Formatter::formatDuration(d.subValue.durationMs, b, s);
  }
  static void fmtDistance(const DisplayData &d, char *b, size_t s) {
    Formatter::formatDistance(d.subValue.distanceKm, b, s);
  }
  static void fmtClock(const DisplayData &d, char *b, size_t s) {
    (void)s;
    Formatter::formatClock(d.subValue.clockTime.hour, d.subValue.clockTime.minute, b);
  }

  DisplayFrame createFrame(const DisplayData &data) const {
    DisplayFrame frame;

    static const char *FIX_LABELS[] = {"WAIT", "2D", "3D"};
    int                fixIdx       = (int)data.fixMode;
    if (fixIdx < 0 || fixIdx > 2) fixIdx = 0;
    frame.header.fixStatus = FIX_LABELS[fixIdx];

    frame.header.modeSpeed = data.modeSpeedLabel;
    frame.header.modeTime  = data.modeTimeLabel;

    Formatter::formatSpeed(data.mainValue, frame.main.value, sizeof(frame.main.value));
    frame.main.unit = data.mainUnit;

    if (data.shouldBlink) {
      strcpy(frame.sub.value, "");
      frame.sub.unit = "";
    } else {
      static const FormatterFunc formatters[] = {fmtDuration, fmtDistance, fmtClock};
      formatters[(int)data.subType](data, frame.sub.value, sizeof(frame.sub.value));
      frame.sub.unit = data.subUnit;
    }
    return frame;
  }
};
