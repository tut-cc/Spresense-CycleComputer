#pragma once

#include "Formatter.h"
#include "ModeState.h"

class SpdTimeState : public ModeState {
public:
  void reset(Trip &trip, DataStore &dataStore) override {
    trip.resetTime();
  }

  void fillFrame(Frame &frame, const Trip &trip, const Clock &clock) const override {
    strcpy(frame.header.modeSpeed, "SPD");
    strcpy(frame.header.modeTime, "Time");
    Formatter::formatSpeed(trip.speedometer.getCur(), frame.main.value, sizeof(frame.main.value));
    strcpy(frame.main.unit, "km/h");
    Formatter::formatDuration(trip.stopwatch.getElapsedTimeMs(), frame.sub.value,
                              sizeof(frame.sub.value));
    strcpy(frame.sub.unit, "");
  }
};

class AvgOdoState : public ModeState {
public:
  void reset(Trip &trip, DataStore &dataStore) override {
    dataStore.clear();
  }

  void fillFrame(Frame &frame, const Trip &trip, const Clock &clock) const override {
    strcpy(frame.header.modeSpeed, "AVG");
    strcpy(frame.header.modeTime, "Odo");
    Formatter::formatSpeed(trip.speedometer.getAvg(), frame.main.value, sizeof(frame.main.value));
    strcpy(frame.main.unit, "km/h");
    Formatter::formatDistance(trip.odometer.getTotalDistance(), frame.sub.value,
                              sizeof(frame.sub.value));
    strcpy(frame.sub.unit, "km");
  }
};

class MaxClockState : public ModeState {
public:
  void reset(Trip &trip, DataStore &dataStore) override {
    // Do nothing
  }

  void fillFrame(Frame &frame, const Trip &trip, const Clock &clock) const override {
    strcpy(frame.header.modeSpeed, "MAX");
    strcpy(frame.header.modeTime, "Clock");
    Formatter::formatSpeed(trip.speedometer.getMax(), frame.main.value, sizeof(frame.main.value));
    strcpy(frame.main.unit, "km/h");
    Formatter::formatTime(clock.getTime(), frame.sub.value, sizeof(frame.sub.value));
    strcpy(frame.sub.unit, "");
  }
};
