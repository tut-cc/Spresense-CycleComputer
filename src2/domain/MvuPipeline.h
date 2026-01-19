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
  if (event == Input::Event::RESET_LONG) { return ResetType::AllWithStorage; }

  if (event == Input::Event::RESET) {
    static const ResetType RESET_MAP[] = {
        ResetType::Trip,     // SPD_TIM
        ResetType::All,      // AVG_ODO
        ResetType::MaxSpeed, // MAX_CLK
    };
    return RESET_MAP[(int)currentMode];
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

  struct ModeConfig {
    const char          *speedLabel;
    const char          *timeLabel;
    const char          *mainUnit;
    const char          *subUnit;
    DisplayData::SubType subType;
  };

  static const ModeConfig CONFIGS[] = {
      {"SPD", "Time", "km/h", "", DisplayData::SubType::Duration},  // SPD_TIM
      {"AVG", "Odo", "km/h", "km", DisplayData::SubType::Distance}, // AVG_ODO
      {"MAX", "Clock", "km/h", "", DisplayData::SubType::Clock}     // MAX_CLK
  };

  const ModeConfig &cfg = CONFIGS[(int)mode];

  data.modeSpeedLabel = cfg.speedLabel;
  data.modeTimeLabel  = cfg.timeLabel;
  data.mainValue      = 0.0f; // Default init
  data.mainUnit       = cfg.mainUnit;
  data.subType        = cfg.subType;
  data.subUnit        = cfg.subUnit;

  switch (mode) {
  case Mode::SPD_TIM:
    data.mainValue           = state.currentSpeed;
    data.subValue.durationMs = state.totalElapsedMs;
    break;

  case Mode::AVG_ODO:
    data.mainValue           = state.avgSpeed;
    data.subValue.distanceKm = state.totalKm;
    break;

  case Mode::MAX_CLK:
    data.mainValue = state.maxSpeed;
    int hour       = currentTime.hour;
    if (currentTime.year >= 2026) hour = (hour + 9) % 24;
    data.subValue.clockTime.hour   = hour;
    data.subValue.clockTime.minute = currentTime.minute;
    break;
  }

  return data;
}

inline SaveData createSaveData(const TripStateData &state, float voltage) {
  SaveData data;
  data.magicNumber   = SAVE_DATA_MAGIC_NUMBER;
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
