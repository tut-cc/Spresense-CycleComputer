#pragma once

#include "../common/DataStructures.h"
#include "../hardware/Gnss.h"
#include "../ui/Input.h"

namespace Pipeline {

inline GnssData collectGnss(Gnss &gnss) {
  GnssData data;
  data.status    = gnss.update() ? UpdateStatus::Updated : UpdateStatus::NoChange;
  data.navData   = gnss.getNavData();
  data.timestamp = millis();
  return data;
}

enum class ResetType { None, Trip, MaxSpeed, All, AllWithStorage };

inline ResetType determineResetType(Input::Event event, Mode currentMode) {
  switch (event) {
  case Input::Event::RESET_LONG:
    return ResetType::AllWithStorage;
  case Input::Event::RESET:
    switch (currentMode) {
    case Mode::SPD_TIM:
      return ResetType::Trip;
    case Mode::AVG_ODO:
      return ResetType::All;
    case Mode::MAX_CLK:
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

inline Mode switchMode(Mode currentMode, Input::Event event) {
  if (event == Input::Event::SELECT) {
    return static_cast<Mode>((static_cast<int>(currentMode) + 1) % 3);
  }
  return currentMode;
}

struct UserInputResult {
  Mode newMode;
  bool shouldClearStorage;
};

template <typename T>
inline UserInputResult handleUserInput(T &state, Mode currentMode, Input::Event event) {
  UserInputResult result = {currentMode, false};
  if (event == Input::Event::NONE) return result;

  result.newMode = switchMode(currentMode, event);
  if (result.newMode != currentMode) state.forceUpdate();

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

inline DisplayData createDisplayData(const TripStateData &state, const GnssData &gnss,
                                     const SpGnssTime &currentTime, Mode mode) {
  DisplayData data;
  data.fixMode            = (SpFixMode)gnss.navData.posFixMode;
  const bool isBlinkPhase = state.isPaused() && ((millis() / 500) % 2 == 0);
  data.shouldBlink        = (mode == Mode::SPD_TIM) && isBlinkPhase;
  data.updateStatus       = state.updateStatus;

  switch (mode) {
  case Mode::SPD_TIM:
    data.modeSpeedLabel      = "SPD";
    data.modeTimeLabel       = "Time";
    data.mainValue           = state.currentSpeed;
    data.mainUnit            = "km/h";
    data.subType             = DisplayData::SubType::Duration;
    data.subValue.durationMs = state.totalElapsedMs;
    data.subUnit             = "";
    break;

  case Mode::AVG_ODO:
    data.modeSpeedLabel      = "AVG";
    data.modeTimeLabel       = "Odo";
    data.mainValue           = state.avgSpeed;
    data.mainUnit            = "km/h";
    data.subType             = DisplayData::SubType::Distance;
    data.subValue.distanceKm = state.totalKm;
    data.subUnit             = "km";
    break;

  case Mode::MAX_CLK:
    data.modeSpeedLabel = "MAX";
    data.modeTimeLabel  = "Clock";
    data.mainValue      = state.maxSpeed;
    data.mainUnit       = "km/h";
    data.subType        = DisplayData::SubType::Clock;

    int hour = currentTime.hour;
    if (currentTime.year >= 2026) { hour = (hour + 9) % 24; }
    data.subValue.clockTime.hour   = hour;
    data.subValue.clockTime.minute = currentTime.minute;
    data.subUnit                   = "";
    break;
  }

  return data;
}

inline SaveData createSaveData(const TripStateData &state, float voltage) {
  SaveData data;
  data.magicNumber   = 0; // Filled by DataStore
  data.totalDistance = state.totalKm;
  data.tripDistance  = state.tripDistance;
  data.movingTimeMs  = state.totalMovingMs;
  data.maxSpeed      = state.maxSpeed;
  data.voltage       = voltage;
  data.updateStatus  = state.updateStatus;
  data.crc           = 0; // Filled by DataStore
  return data;
}

} // namespace Pipeline
