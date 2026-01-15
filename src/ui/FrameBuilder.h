#pragma once

#include <GNSS.h>
#include <cstring>

#include "../domain/Clock.h"
#include "../domain/Trip.h"
#include "Formatter.h"
#include "Frame.h"
#include "Mode.h"

class FrameBuilder {
public:
  static Frame build(const Trip &trip, const Clock &clock, Mode::ID modeId, SpFixMode fixMode) {
    Frame frame;

    // Set Header Fix Status
    switch (fixMode) {
    case FixInvalid:
      strcpy(frame.header.fixStatus, "WAIT");
      break;
    case Fix2D:
      strcpy(frame.header.fixStatus, "2D");
      break;
    case Fix3D:
      strcpy(frame.header.fixStatus, "3D");
      break;
    default:
      strcpy(frame.header.fixStatus, "");
      break;
    }

    // Set Mode Data
    switch (modeId) {
    case Mode::ID::SPD_TIME:
      strcpy(frame.header.modeSpeed, "SPD");
      strcpy(frame.header.modeTime, "Time");
      Formatter::formatSpeed(trip.speedometer.getCur(), frame.main.value, sizeof(frame.main.value));
      strcpy(frame.main.unit, "km/h");
      Formatter::formatDuration(trip.stopwatch.getElapsedTimeMs(), frame.sub.value,
                                sizeof(frame.sub.value));
      strcpy(frame.sub.unit, "");
      break;
    case Mode::ID::AVG_ODO:
      strcpy(frame.header.modeSpeed, "AVG");
      strcpy(frame.header.modeTime, "Odo");
      Formatter::formatSpeed(trip.speedometer.getAvg(), frame.main.value, sizeof(frame.main.value));
      strcpy(frame.main.unit, "km/h");
      Formatter::formatDistance(trip.odometer.getTotalDistance(), frame.sub.value,
                                sizeof(frame.sub.value));
      strcpy(frame.sub.unit, "km");
      break;
    case Mode::ID::MAX_CLOCK:
      strcpy(frame.header.modeSpeed, "MAX");
      strcpy(frame.header.modeTime, "Clock");
      Formatter::formatSpeed(trip.speedometer.getMax(), frame.main.value, sizeof(frame.main.value));
      strcpy(frame.main.unit, "km/h");
      Formatter::formatTime(clock.getTime(), frame.sub.value, sizeof(frame.sub.value));
      strcpy(frame.sub.unit, "");
      break;
    default:
      strcpy(frame.main.value, "ERROR");
      strcpy(frame.main.unit, "");
      strcpy(frame.sub.value, "");
      strcpy(frame.sub.unit, "");
      break;
    }

    return frame;
  }
};
