#pragma once

#include <GNSS.h>
#include <cstring>

#include "../domain/Clock.h"
#include "../domain/Trip.h"
#include "Formatter.h"
#include "Mode.h"

struct Frame {
  char mainValue[16] = "";
  char mainUnit[16]  = "";
  char subValue[16]  = "";
  char subUnit[16]   = "";
  char fixStatus[8]  = "";

  Frame() = default;

  bool operator==(Frame &other) const {
    const bool mainValueEq = strcmp(mainValue, other.mainValue) == 0;
    const bool mainUnitEq  = strcmp(mainUnit, other.mainUnit) == 0;
    const bool subValueEq  = strcmp(subValue, other.subValue) == 0;
    const bool subUnitEq   = strcmp(subUnit, other.subUnit) == 0;
    const bool fixStatusEq = strcmp(fixStatus, other.fixStatus) == 0;
    return mainValueEq && mainUnitEq && subValueEq && subUnitEq && fixStatusEq;
  }

  Frame(Trip &trip, Clock &clock, Mode::ID modeId, SpFixMode fixMode) {
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
      strcpy(fixStatus, "");
      break;
    }
  }

private:
  void getModeData(Trip &trip, Clock &clock, Mode::ID modeId) {
    switch (modeId) {
    case Mode::ID::SPEED_TIME:
      Formatter::formatSpeed(trip.speedometer.getCur(), mainValue, sizeof(mainValue));
      strcpy(mainUnit, "km/h");
      Formatter::formatDuration(trip.stopwatch.getElapsedTimeMs(), subValue, sizeof(subValue));
      strcpy(subUnit, "");
      break;
    case Mode::ID::AVG_ODO:
      Formatter::formatSpeed(trip.speedometer.getAvg(), mainValue, sizeof(mainValue));
      strcpy(mainUnit, "km/h");
      Formatter::formatDistance(trip.odometer.getTotalDistance(), subValue, sizeof(subValue));
      strcpy(subUnit, "km");
      break;
    case Mode::ID::MAX_CLOCK:
      Formatter::formatSpeed(trip.speedometer.getMax(), mainValue, sizeof(mainValue));
      strcpy(mainUnit, "km/h");
      Formatter::formatTime(clock.getTime(), subValue, sizeof(subValue));
      strcpy(subUnit, "");
      break;
    default:
      strcpy(mainValue, "ERROR");
      strcpy(mainUnit, "");
      strcpy(subValue, "");
      strcpy(subUnit, "");
      break;
    }
  }
};
