#pragma once

#include <GNSS.h>
#include <cstring>

#include "../domain/Clock.h"
#include "../domain/Trip.h"
#include "Formatter.h"
#include "Mode.h"

struct Frame {
  struct Item {
    char value[16] = "";
    char unit[16]  = "";

    bool operator==(const Item &other) const {
      return strcmp(value, other.value) == 0 && strcmp(unit, other.unit) == 0;
    }
  };

  struct Header {
    char fixStatus[8] = "";
    char modeSpeed[8] = "";
    char modeTime[8]  = "";

    bool operator==(const Header &other) const {
      const bool fixStatusEq = strcmp(fixStatus, other.fixStatus) == 0;
      const bool modeSpeedEq = strcmp(modeSpeed, other.modeSpeed) == 0;
      const bool modeTimeEq  = strcmp(modeTime, other.modeTime) == 0;
      return fixStatusEq && modeSpeedEq && modeTimeEq;
    }
  };

  Header header;
  Item   main;
  Item   sub;

  Frame() = default;

  bool operator==(const Frame &other) const {
    return header == other.header && main == other.main && sub == other.sub;
  }

  Frame(Trip &trip, Clock &clock, Mode::ID modeId, SpFixMode fixMode) {
    getModeData(trip, clock, modeId);

    switch (fixMode) {
    case FixInvalid:
      strcpy(header.fixStatus, "WAIT");
      break;
    case Fix2D:
      strcpy(header.fixStatus, "2D");
      break;
    case Fix3D:
      strcpy(header.fixStatus, "3D");
      break;
    default:
      strcpy(header.fixStatus, "");
      break;
    }
  }

private:
  void getModeData(Trip &trip, Clock &clock, Mode::ID modeId) {
    switch (modeId) {
    case Mode::ID::SPEED_TIME:
      strcpy(header.modeSpeed, "SPD");
      strcpy(header.modeTime, "Time");
      Formatter::formatSpeed(trip.speedometer.getCur(), main.value, sizeof(main.value));
      strcpy(main.unit, "km/h");
      Formatter::formatDuration(trip.stopwatch.getElapsedTimeMs(), sub.value, sizeof(sub.value));
      strcpy(sub.unit, "");
      break;
    case Mode::ID::AVG_ODO:
      strcpy(header.modeSpeed, "AVG");
      strcpy(header.modeTime, "Odo");
      Formatter::formatSpeed(trip.speedometer.getAvg(), main.value, sizeof(main.value));
      strcpy(main.unit, "km/h");
      Formatter::formatDistance(trip.odometer.getTotalDistance(), sub.value, sizeof(sub.value));
      strcpy(sub.unit, "km");
      break;
    case Mode::ID::MAX_CLOCK:
      strcpy(header.modeSpeed, "MAX");
      strcpy(header.modeTime, "Clock");
      Formatter::formatSpeed(trip.speedometer.getMax(), main.value, sizeof(main.value));
      strcpy(main.unit, "km/h");
      Formatter::formatTime(clock.getTime(), sub.value, sizeof(sub.value));
      strcpy(sub.unit, "");
      break;
    default:
      strcpy(main.value, "ERROR");
      strcpy(main.unit, "");
      strcpy(sub.value, "");
      strcpy(sub.unit, "");
      break;
    }
  }
};
