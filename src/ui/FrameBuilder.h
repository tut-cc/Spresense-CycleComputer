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

    Clock::Time t = clock.getTime();
    Formatter::formatTime(t.hour, t.minute, frame.footerTime, sizeof(frame.footerTime));
  }

private:
  void getModeData(Frame &frame, const Trip &trip, const Clock &clock, Mode::ID modeId) {
    switch (modeId) {
    case Mode::ID::SPEED:
      Formatter::formatSpeed(trip.speedometer.get(), frame.value, sizeof(frame.value));
      strcpy(frame.unit, "km/h");
      strcpy(frame.footerMode, "SPEED");
      break;
    case Mode::ID::MAX_SPEED:
      Formatter::formatSpeed(trip.speedometer.getMax(), frame.value, sizeof(frame.value));
      strcpy(frame.unit, "km/h");
      strcpy(frame.footerMode, "MAX SPEED");
      break;
    case Mode::ID::AVG_SPEED:
      Formatter::formatSpeed(trip.getAvgSpeedKmh(), frame.value, sizeof(frame.value));
      strcpy(frame.unit, "km/h");
      strcpy(frame.footerMode, "AVG SPEED");
      break;
    case Mode::ID::DISTANCE:
      Formatter::formatDistance(trip.odometer.getDistance(), frame.value, sizeof(frame.value));
      strcpy(frame.unit, "km");
      strcpy(frame.footerMode, "DISTANCE");
      break;
    case Mode::ID::TIME: {
      Clock::Time t = clock.getTime();
      Formatter::formatTime(t.hour, t.minute, frame.value, sizeof(frame.value));
      frame.unit[0] = '\0';
      strcpy(frame.footerMode, "TIME");
      break;
    }
    case Mode::ID::MOVING_TIME:
      Formatter::formatDuration(trip.stopwatch.getMovingTimeMs(), frame.value, sizeof(frame.value));
      frame.unit[0] = '\0';
      strcpy(frame.footerMode, "TRIP TIME");
      break;
    case Mode::ID::ELAPSED_TIME:
      Formatter::formatDuration(trip.stopwatch.getElapsedTimeMs(), frame.value, sizeof(frame.value));
      frame.unit[0] = '\0';
      strcpy(frame.footerMode, "ELAPSED TIME");
      break;
    }
  }
};
