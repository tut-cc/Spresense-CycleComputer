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
  void build(Frame &frame, const Trip &trip, const Clock &clock, Mode::ID modeId, SpFixMode fixMode, int numSatellites) {
    getModeData(frame, trip, clock, modeId);

    switch (fixMode) {
    case FixInvalid:
      strcpy(frame.fixStatus, "WAIT");
      break;
    case Fix2D:
      strcpy(frame.fixStatus, "2D");
      break;
    case Fix3D:
      strcpy(frame.fixStatus, "3D");
      break;
    default:
      frame.fixStatus[0] = '\0';
      break;
    }

    snprintf(frame.satelliteCount, sizeof(frame.satelliteCount), "St:%d", numSatellites);
  }

private:
  void getModeData(Frame &frame, const Trip &trip, const Clock &clock, Mode::ID modeId) {
    switch (modeId) {
    case Mode::ID::SPEED:
      strcpy(frame.title, "SPEED");
      Formatter::formatSpeed(trip.speedometer.get(), frame.value, sizeof(frame.value));
      strcpy(frame.unit, "km/h");
      break;
    case Mode::ID::MAX_SPEED:
      strcpy(frame.title, "MAX SPEED");
      Formatter::formatSpeed(trip.speedometer.getMax(), frame.value, sizeof(frame.value));
      strcpy(frame.unit, "km/h");
      break;
    case Mode::ID::AVG_SPEED:
      strcpy(frame.title, "AVG SPEED");
      Formatter::formatSpeed(trip.getAvgSpeedKmh(), frame.value, sizeof(frame.value));
      strcpy(frame.unit, "km/h");
      break;
    case Mode::ID::DISTANCE:
      strcpy(frame.title, "DISTANCE");
      Formatter::formatDistance(trip.odometer.getDistance(), frame.value, sizeof(frame.value));
      strcpy(frame.unit, "km");
      break;
    case Mode::ID::TIME: {
      strcpy(frame.title, "TIME");
      Clock::Time t = clock.getTime();
      Formatter::formatTime(t.hour, t.minute, frame.value, sizeof(frame.value));
      frame.unit[0] = '\0';
      break;
    }
    case Mode::ID::MOVING_TIME:
      strcpy(frame.title, "TRIP TIME");
      Formatter::formatDuration(trip.stopwatch.getMovingTimeMs(), frame.value, sizeof(frame.value));
      frame.unit[0] = '\0';
      break;
    case Mode::ID::ELAPSED_TIME:
      strcpy(frame.title, "ELAPSED TIME");
      Formatter::formatDuration(trip.stopwatch.getElapsedTimeMs(), frame.value, sizeof(frame.value));
      frame.unit[0] = '\0';
      break;
    }
  }
};
