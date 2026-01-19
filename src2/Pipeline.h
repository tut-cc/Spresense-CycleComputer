#pragma once

#include "DataStructures.h"
#include "TripCompute.h"
#include "hardware/Gnss.h"

#include "ui/Input.h"
#include "ui/Mode.h"

namespace Pipeline {

// ========================================
// Stage 1: GNSS入力収集
// ========================================

inline GnssData collectGnss(Gnss &gnss) {
  GnssData data;
  data.status    = gnss.update() ? UpdateStatus::Updated : UpdateStatus::NoChange;
  data.navData   = gnss.getNavData();
  data.timestamp = millis();
  return data;
}

// Stage 2: Trip計算はTripCompute.hで定義

// ========================================
// Stage 3: ユーザー入力処理
// ========================================

enum class ResetType {
  None,
  Trip,          // トリップデータのみ
  MaxSpeed,      // 最高速度のみ
  All,           // 全データ
  AllWithStorage // 全データ + EEPROM
};

// リセットタイプを決定
inline ResetType determineResetType(Input::Event event, Mode::ID currentMode) {
  switch (event) {
  case Input::Event::RESET_LONG:
    return ResetType::AllWithStorage;

  case Input::Event::RESET:
    switch (currentMode) {
    case Mode::ID::SPD_TIM:
      return ResetType::Trip;
    case Mode::ID::AVG_ODO:
      return ResetType::All;
    case Mode::ID::MAX_CLK:
      return ResetType::MaxSpeed;
    }
    break;

  default:
    return ResetType::None;
  }
  return ResetType::None;
}

// リセット操作を適用
template <typename T> inline T applyReset(const T &state, ResetType resetType) {
  T newState = state;

  switch (resetType) {
  case ResetType::None:
    return state; // 変更なし

  case ResetType::Trip:
    // トリップデータのみリセット
    newState.totalElapsedMs = 0;
    newState.tripDistance   = 0.0f;
    newState.currentSpeed   = 0.0f;
    newState.avgSpeed       = 0.0f;
    newState.totalMovingMs  = 0;
    newState.status         = TripStateData::Status::Stopped;
    newState.updateStatus   = UpdateStatus::ForceUpdate;
    break;

  case ResetType::MaxSpeed:
    // 最高速度のみリセット
    newState.maxSpeed     = 0.0f;
    newState.updateStatus = UpdateStatus::ForceUpdate;
    break;

  case ResetType::All:
  case ResetType::AllWithStorage:
    // 全データリセット
    newState.totalElapsedMs = 0;
    newState.tripDistance   = 0.0f;
    newState.currentSpeed   = 0.0f;
    newState.maxSpeed       = 0.0f;
    newState.avgSpeed       = 0.0f;
    newState.totalMovingMs  = 0;
    newState.totalKm        = 0.0f;
    newState.status         = TripStateData::Status::Stopped;
    newState.updateStatus   = UpdateStatus::ForceUpdate;
    break;
  }

  return newState;
}

// Pause操作を適用
template <typename T> inline T applyPause(const T &state) {
  T newState = state;

  if (newState.status == TripStateData::Status::Paused) {
    newState.status = TripStateData::Status::Stopped;
  } else {
    newState.status = TripStateData::Status::Paused;
  }

  newState.updateStatus = UpdateStatus::ForceUpdate;
  return newState;
}

// モード切り替え
inline Mode::ID switchMode(Mode::ID currentMode, Input::Event event) {
  if (event == Input::Event::SELECT) {
    return static_cast<Mode::ID>((static_cast<int>(currentMode) + 1) % 3);
  }
  return currentMode;
}

// ユーザー入力処理の統合
template <typename T> struct UserInputResult {
  T        newState;
  Mode::ID newMode;
  bool     shouldClearStorage; // EEPROM消去が必要か
};

template <typename T>
inline UserInputResult<T> handleUserInput(const T &state, Mode::ID currentMode,
                                          Input::Event event) {
  UserInputResult<T> result;
  result.newState           = state;
  result.newMode            = currentMode;
  result.shouldClearStorage = false;

  if (event == Input::Event::NONE) return result;

  // モード切り替え
  result.newMode = switchMode(currentMode, event);
  if (result.newMode != currentMode) { result.newState.updateStatus = UpdateStatus::ForceUpdate; }

  // Pause処理
  if (event == Input::Event::PAUSE) {
    result.newState = applyPause(state);
    return result;
  }

  // リセット処理
  ResetType resetType       = determineResetType(event, currentMode);
  result.newState           = applyReset(state, resetType);
  result.shouldClearStorage = (resetType == ResetType::AllWithStorage);

  return result;
}

// ========================================
// Stage 4: 表示データ生成
// ========================================

inline DisplayData createDisplayData(const TripStateData &state, const GnssData &gnss,
                                     Mode::ID mode) {
  DisplayData data;
  data.fixMode     = (SpFixMode)gnss.navData.posFixMode;
  data.shouldBlink = (state.status == TripStateData::Status::Paused) && ((millis() / 500) % 2 == 0);
  data.updateStatus = state.updateStatus;

  switch (mode) {
  case Mode::ID::SPD_TIM:
    data.modeSpeedLabel      = "SPD";
    data.modeTimeLabel       = "Time";
    data.mainValue           = state.currentSpeed;
    data.mainUnit            = "km/h";
    data.subType             = DisplayData::SubType::Duration;
    data.subValue.durationMs = state.totalElapsedMs;
    data.subUnit             = "";
    break;

  case Mode::ID::AVG_ODO:
    data.modeSpeedLabel      = "AVG";
    data.modeTimeLabel       = "Odo";
    data.mainValue           = state.avgSpeed;
    data.mainUnit            = "km/h";
    data.subType             = DisplayData::SubType::Distance;
    data.subValue.distanceKm = state.totalKm;
    data.subUnit             = "km";
    break;

  case Mode::ID::MAX_CLK:
    data.modeSpeedLabel = "MAX";
    data.modeTimeLabel  = "Clock";
    data.mainValue      = state.maxSpeed;
    data.mainUnit       = "km/h";
    data.subType        = DisplayData::SubType::Clock;

    // JSTへの時間変換 (もともとClock.hにあったロジックを統合)
    int hour = gnss.navData.time.hour;
    if (gnss.navData.time.year >= 2026) {
      hour = (hour + 9) % 24; // JST Offset +9
    }
    data.subValue.clockTime.hour   = hour;
    data.subValue.clockTime.minute = gnss.navData.time.minute;
    data.subUnit                   = "";
    break;
  }

  return data;
}

// ========================================
// Stage 5: 永続化データ生成
// ========================================

inline PersistentData createPersistentData(const TripStateData &state, float voltage) {
  PersistentData data;
  data.totalDistance = state.totalKm;
  data.tripDistance  = state.tripDistance;
  data.movingTimeMs  = state.totalMovingMs;
  data.maxSpeed      = state.maxSpeed;
  data.voltage       = voltage;
  data.updateStatus  = state.updateStatus;
  return data;
}

} // namespace Pipeline
