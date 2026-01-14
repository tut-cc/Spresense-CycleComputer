#pragma once

#include <GNSS.h>
#include <cstring>

#include "../domain/Clock.h"
#include "../domain/Trip.h"
#include "Formatter.h"
#include "Mode.h"

struct Frame {
  char value[32]         = "";
  char unit[16]          = "";
  char fixStatus[8]      = "";
  char satelliteCount[8] = "";
  char footerTime[16]    = "";
  char footerMode[16]    = "";

  Frame() = default;

  bool operator==(Frame &other) const {
    const bool valueEq          = strcmp(value, other.value) == 0;
    const bool unitEq           = strcmp(unit, other.unit) == 0;
    const bool fixStatusEq      = strcmp(fixStatus, other.fixStatus) == 0;
    const bool satelliteCountEq = strcmp(satelliteCount, other.satelliteCount) == 0;
    const bool footerTimeEq     = strcmp(footerTime, other.footerTime) == 0;
    const bool footerModeEq     = strcmp(footerMode, other.footerMode) == 0;
    return valueEq && unitEq && fixStatusEq && satelliteCountEq && footerTimeEq && footerModeEq;
  }

  Frame(Trip &trip, Clock &clock, Mode::ID modeId, SpFixMode fixMode, int numSatellites) {
    getModeData(trip, clock, modeId);

    switch (fixMode) {
    case FixInvalid:
      strcpy(fixStatus, "WAIT");
      break;
    case Fix2D:
      strcpy(fixStatus, "2D");
      break;
    case Fix3D:
      strcpy(fixStatus, "3D");
      break;
    default:
      fixStatus[0] = '\0';
      break;
    }

    snprintf(satelliteCount, sizeof(satelliteCount), "St:%d", numSatellites);
    Formatter::formatTime(clock.getTime(), footerTime, sizeof(footerTime));
  }

private:
  void getModeData(Trip &trip, Clock &clock, Mode::ID modeId) {
    switch (modeId) {
    case Mode::ID::SPEED:
      Formatter::formatSpeed(trip.speedometer.getCur(), value, sizeof(value));
      strcpy(unit, "km/h");
      strcpy(footerMode, "SPEED");
      break;
    case Mode::ID::MAX_SPEED:
      Formatter::formatSpeed(trip.speedometer.getMax(), value, sizeof(value));
      strcpy(unit, "km/h");
      strcpy(footerMode, "MAX SPEED");
      break;
    case Mode::ID::AVG_SPEED:
      Formatter::formatSpeed(trip.speedometer.getAvg(), value, sizeof(value));
      strcpy(unit, "km/h");
      strcpy(footerMode, "AVG SPEED");
      break;
    case Mode::ID::DISTANCE:
      Formatter::formatDistance(trip.odometer.getDistance(), value, sizeof(value));
      strcpy(unit, "km");
      strcpy(footerMode, "DISTANCE");
      break;
    case Mode::ID::TIME: {
      Formatter::formatTime(clock.getTime(), value, sizeof(value));
      strcpy(unit, "");
      strcpy(footerMode, "TIME");
      break;
    }
    case Mode::ID::MOVING_TIME:
      Formatter::formatDuration(trip.stopwatch.getMovingTimeMs(), value, sizeof(value));
      strcpy(unit, "");
      strcpy(footerMode, "TRIP TIME");
      break;
    case Mode::ID::ELAPSED_TIME:
      Formatter::formatDuration(trip.stopwatch.getElapsedTimeMs(), value, sizeof(value));
      strcpy(unit, "");
      strcpy(footerMode, "ELAPSED TIME");
      break;
    }
  }
};
