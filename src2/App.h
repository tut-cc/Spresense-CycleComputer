#pragma once

#include <Arduino.h>
#include <stddef.h>

#include "common/DoubleBuffer.h"
#include "domain/DataStore.h"
#include "domain/TripLogic.h"
#include "domain/VoltageMonitor.h"
#include "hardware/Clock.h"
#include "hardware/Gnss.h"
#include "ui/FrameLogic.h"
#include "ui/UI.h"

class App {
private:
  Gnss           gnss;
  Clock          systemClock;
  DataStore      dataStore;
  VoltageMonitor voltageMonitor;
  UI             userInterface;

  Mode currentMode = Mode::SPD_TIM;

  DoubleBuffer<TripState>    tripBuffer;
  DoubleBuffer<DisplayFrame> frameBuffer;
  DoubleBuffer<SaveData>     saveBuffer;

  unsigned long currentTime    = 0;
  GnssData      currentGnss    = {};
  Input::Event  currentButton  = Input::Event::NONE;
  SpGnssTime    currentClock   = {};
  float         currentVoltage = 0.0f;

  unsigned long lastSaveMs     = 0;
  unsigned long lastUiUpdateMs = 0;

public:
  void begin() {
    gnss.begin();
    systemClock.begin();
    voltageMonitor.begin();
    userInterface.begin();
    loadFromStorage();
  }

  void update() {
    collectInputs();
    updateState();
    processOutputs();
  }

private:
  void loadFromStorage() {
    SaveData saved = dataStore.load();

    TripState state;
    state.resetAll();
    state.distance.total = saved.totalDistance;
    state.distance.trip  = saved.tripDistance;
    state.time.moving    = saved.movingTimeMs;
    state.speed.max      = saved.maxSpeed;

    tripBuffer.initialize(state);
    saveBuffer.initialize(saved);
    lastSaveMs = millis();
  }

  void collectInputs() {
    currentTime    = millis();
    currentButton  = userInterface.getInputEvent();
    currentClock   = systemClock.now();
    currentVoltage = voltageMonitor.update();

    bool updated        = gnss.update();
    currentGnss.status  = updated ? UpdateStatus::Updated : UpdateStatus::NoChange;
    currentGnss.navData = gnss.navData;

    if (updated && (SpFixMode)currentGnss.navData.posFixMode != FixInvalid) {
      systemClock.sync(currentGnss.navData.time);
    }
  }

  void updateState() {
    tripBuffer.prepare();
    tripBuffer.current().resetMeta();

    if (currentButton != Input::Event::NONE) {
      if (handleButton()) return;
    }

    TripLogic::computeTrip(tripBuffer.current(), currentGnss, currentTime);
  }

  bool handleButton() {
    TripState &state = tripBuffer.current();

    switch (currentButton) {
    case Input::Event::SELECT:
      currentMode = static_cast<Mode>((static_cast<int>(currentMode) + 1) % 3);
      state.forceUpdate();
      break;

    case Input::Event::PAUSE:
      state.status =
          state.isPaused() ? TripStateBase::Status::Stopped : TripStateBase::Status::Paused;
      state.forceUpdate();
      break;

    case Input::Event::RESET:
      applyReset(currentMode);
      break;

    case Input::Event::RESET_LONG:
      state.resetAll();
      dataStore.clear();
      userInterface.showResetMessage();
      frameBuffer.initialize(DisplayFrame());
      saveBuffer.initialize(createSaveData(state, 0.0f));
      return true;

    default:
      break;
    }
    return false;
  }

  void applyReset(Mode mode) {
    TripState &state = tripBuffer.current();
    switch (mode) {
    case Mode::SPD_TIM:
      state.resetTrip();
      break;
    case Mode::AVG_ODO:
      state.resetAll();
      break;
    case Mode::MAX_CLK:
      state.resetMaxSpeed();
      break;
    }
  }

  void processOutputs() {
    outputToDisplay();
    outputToStorage();
  }

  void outputToDisplay() {
    if (!shouldUpdateUI()) return;

    DisplayFrame nextFrame =
        FrameLogic::buildFrame(tripBuffer.current(), currentGnss, currentClock, currentMode);

    if (frameBuffer.apply(nextFrame)) {
      userInterface.draw(frameBuffer.current());
      lastUiUpdateMs = currentTime;
    }
  }

  bool shouldUpdateUI() const {
    return (currentButton != Input::Event::NONE) || (currentTime - lastUiUpdateMs >= 500) ||
           TripLogic::isChanged(tripBuffer.previous(), tripBuffer.current()) ||
           (currentGnss.status == UpdateStatus::Updated);
  }

  void outputToStorage() {
    if (!shouldSave()) return;

    SaveData nextSave = createSaveData(tripBuffer.current(), currentVoltage);

    if (saveBuffer.apply(nextSave)) { dataStore.save(saveBuffer.current()); }
    lastSaveMs = currentTime;
  }

  bool shouldSave() const {
    const bool shouldUpdate = (currentTime - lastSaveMs >= DataStore::SAVE_INTERVAL_MS);
    const bool gnssStable   = (currentGnss.status == UpdateStatus::NoChange);
    return shouldUpdate && gnssStable;
  }

  static SaveData createSaveData(const TripState &state, float voltage) {
    SaveData data;
    data.magicNumber   = SAVE_DATA_MAGIC_NUMBER;
    data.totalDistance = state.distance.total;
    data.tripDistance  = state.distance.trip;
    data.movingTimeMs  = state.time.moving;
    data.maxSpeed      = state.speed.max;
    data.voltage       = voltage;
    data.updateStatus  = state.updateStatus;
    data.crc           = 0;
    return data;
  }
};
