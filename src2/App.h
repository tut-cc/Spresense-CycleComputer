#pragma once

/**
 * @file App.h
 * @brief サイクルコンピュータのメインアプリケーションクラス
 *
 * 全モジュールを統合し、メインループを制御します。
 * 入力収集 → 状態更新 → 出力処理 のパイプラインで動作。
 */

#include <Arduino.h>
#include <stddef.h>

#include "common/Config.h"
#include "common/DoubleBuffer.h"
#include "domain/DataStore.h"
#include "domain/TripState.h"
#include "domain/VoltageMonitor.h"
#include "hardware/Clock.h"
#include "hardware/Gnss.h"
#include "ui/DisplayFrame.h"
#include "ui/Input.h"
#include "ui/Renderer.h"
#include <LowPower.h>

class App {
private:
  Gnss           gnss;
  Clock          systemClock;
  DataStore      dataStore;
  VoltageMonitor voltageMonitor;
  Input          input;
  Renderer       renderer;

  Mode currentMode = Mode::SPD_TIM;

  DoubleBuffer<TripState>    tripBuffer;
  DoubleBuffer<DisplayFrame> frameBuffer;
  DoubleBuffer<SaveData>     saveBuffer;

  unsigned long currentTime   = 0;
  GnssData      currentGnss   = {};
  Input::Event  currentButton = Input::Event::NONE;

  unsigned long lastSaveMs     = 0;
  unsigned long lastUiUpdateMs = 0;

public:
  App() : input(Config::Pins::BUTTON_SELECT, Config::Pins::BUTTON_PAUSE) {}

  void begin() {
    if (!renderer.begin()) shutdown();
    input.begin();
    if (!gnss.begin()) shutdown();

    systemClock.begin();
    voltageMonitor.begin();
    loadFromStorage();
  }

  void update() {
    collectInputs();
    updateState();
    processOutputs();
  }

private:
  void shutdown() {
    LowPower.begin();
    LowPower.deepSleep(0);
  }

  void loadFromStorage() {
    SaveData saved = dataStore.load();
    tripBuffer.initialize(TripState(saved));
    saveBuffer.initialize(saved);
    lastSaveMs = millis();
  }

  void collectInputs() {
    currentTime   = millis();
    currentButton = input.update();

    bool updated        = gnss.update();
    currentGnss.updated = updated;
    currentGnss.navData = gnss.navData;

    if (updated) {
      const SpFixMode fixMode = (SpFixMode)currentGnss.navData.posFixMode;
      if (fixMode == Fix2D || fixMode == Fix3D) { systemClock.sync(currentGnss.navData.time); }
    }
  }

  void updateState() {
    if (currentButton != Input::Event::NONE) handleButton();
    tripBuffer.apply(TripState(tripBuffer.current(), currentGnss, currentTime));
  }

  void handleButton() {
    TripState &state = tripBuffer.current();

    switch (currentButton) {
    case Input::Event::SELECT:
      currentMode = rotateMode(currentMode);
      break;

    case Input::Event::PAUSE:
      state.status = state.isPaused() ? TripState::Status::Stopped : TripState::Status::Paused;
      break;

    case Input::Event::RESET:
      applyReset(currentMode);
      break;

    case Input::Event::RESET_LONG:
      resetAllData();
      break;

    default:
      break;
    }
  }

  static Mode rotateMode(Mode mode) {
    return static_cast<Mode>((static_cast<int>(mode) + 1) % Config::UI::MODE_COUNT);
  }

  void resetAllData() {
    TripState &state = tripBuffer.current();
    state.clearAllData();
    dataStore.clear();
    renderer.showResetMessage();
    frameBuffer.initialize(DisplayFrame());
    saveBuffer.initialize(SaveData(state, 0.0f));
  }

  void applyReset(Mode mode) {
    TripState &state = tripBuffer.current();
    switch (mode) {
    case Mode::SPD_TIM:
      state.clearTripData();
      break;
    case Mode::AVG_ODO:
      state.clearAllData();
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

    SpGnssTime   nowClock = systemClock.now();
    DisplayFrame nextFrame(tripBuffer.current(), currentGnss, nowClock, currentMode);
    if (frameBuffer.apply(nextFrame)) {
      renderer.render(frameBuffer.current());
      lastUiUpdateMs = currentTime;
    }
  }

  bool shouldUpdateUI() const {
    const bool hasButtonInput  = (currentButton != Input::Event::NONE);
    const bool intervalElapsed = (currentTime - lastUiUpdateMs >= Config::UI::UPDATE_INTERVAL_MS);
    const bool stateChanged    = (tripBuffer.previous() != tripBuffer.current());
    const bool gnssUpdated     = currentGnss.updated;
    return hasButtonInput || intervalElapsed || stateChanged || gnssUpdated;
  }

  void outputToStorage() {
    if (!shouldSave()) return;
    float      currentVoltage = voltageMonitor.update();
    TripState &state          = tripBuffer.current();
    state.updateAverageSpeed();

    SaveData nextSave(state, currentVoltage);
    if (saveBuffer.apply(nextSave)) dataStore.save(saveBuffer.current());
    lastSaveMs = currentTime;
  }

  bool shouldSave() const {
    const bool shouldUpdate = (currentTime - lastSaveMs >= DataStore::SAVE_INTERVAL_MS);
    const bool gnssStable   = !currentGnss.updated;
    return shouldUpdate && gnssStable;
  }
};
