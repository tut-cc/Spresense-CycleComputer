#pragma once

#include "DataStructures.h"
#include "hardware/Gnss.h"

#include "ui/Input.h"
#include "ui/Mode.h"

namespace Pipeline {

// ========================================
// Stage 1: GNSS Capture
// ========================================

inline GnssData collectGnss(Gnss &gnss) {
  GnssData data;
  data.status    = gnss.update() ? UpdateStatus::Updated : UpdateStatus::NoChange;
  data.navData   = gnss.getNavData();
  data.timestamp = millis();
  return data;
}

// Stage 2: Trip Logic (Refer to TripCompute.h)

// ========================================
// Stage 3: User Interaction
// ========================================

enum class ResetType { None, Trip, MaxSpeed, All, AllWithStorage };

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
    break;
  }
  return ResetType::None;
}

template <typename T> inline void applyReset(T &state, ResetType resetType) {
  switch (resetType) {
  case ResetType::Trip:
    state.resetTrip();
    break;
  case ResetType::MaxSpeed:
    state.resetMaxSpeed();
    break;
  case ResetType::All:
  case ResetType::AllWithStorage:
    state.resetAll();
    break;
  default:
    break;
  }
}

inline void applyPause(TripStateData &state) {
  state.status = (state.status == TripStateData::Status::Paused) ? TripStateData::Status::Stopped
                                                                 : TripStateData::Status::Paused;
  state.forceUpdate();
}

inline Mode::ID switchMode(Mode::ID currentMode, Input::Event event) {
  if (event == Input::Event::SELECT) {
    return static_cast<Mode::ID>((static_cast<int>(currentMode) + 1) % 3);
  }
  return currentMode;
}

struct UserInputResult {
  Mode::ID newMode;
  bool     shouldClearStorage;
};

template <typename T>
inline UserInputResult handleUserInput(T &state, Mode::ID currentMode, Input::Event event) {
  UserInputResult result = {currentMode, false};
  if (event == Input::Event::NONE) return result;

  // Mode switching
  result.newMode = switchMode(currentMode, event);
  if (result.newMode != currentMode) { state.forceUpdate(); }

  // Logic for specific events
  switch (event) {
  case Input::Event::PAUSE:
    applyPause(state);
    break;

  case Input::Event::RESET:
  case Input::Event::RESET_LONG: {
    ResetType r = determineResetType(event, currentMode);
    applyReset(state, r);
    result.shouldClearStorage = (r == ResetType::AllWithStorage);
    break;
  }
  default:
    break;
  }

  return result;
}

// ========================================
// Stage 4: View Model Generation
// ========================================

inline DisplayData createDisplayData(const TripStateData &state, const GnssData &gnss,
                                     Mode::ID mode) {
  DisplayData data;
  data.fixMode      = (SpFixMode)gnss.navData.posFixMode;
  data.shouldBlink  = state.isPaused() && ((millis() / 500) % 2 == 0);
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

    int hour = gnss.navData.time.hour;
    if (gnss.navData.time.year >= 2026) { hour = (hour + 9) % 24; }
    data.subValue.clockTime.hour   = hour;
    data.subValue.clockTime.minute = gnss.navData.time.minute;
    data.subUnit                   = "";
    break;
  }

  return data;
}

// Stage 5
inline PersistentData createPersistentData(const TripStateData &state, float voltage) {
  return {state.totalKm, state.tripDistance, state.totalMovingMs, state.maxSpeed,
          voltage,       state.updateStatus};
}

} // namespace Pipeline
