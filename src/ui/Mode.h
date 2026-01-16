#pragma once

#include "../domain/Clock.h"
#include "../domain/DataStore.h"
#include "../domain/Trip.h"
#include "Input.h"
#include "Renderer.h"

class ModeState {
public:
  virtual ~ModeState() = default;

  virtual void onInput(Input::ID id, Trip &trip, DataStore &dataStore)             = 0;
  virtual void fillFrame(Frame &frame, const Trip &trip, const Clock &clock) const = 0;
};

class SpdTimeState : public ModeState {
public:
  void onInput(Input::ID id, Trip &trip, DataStore &) override {
    if (id == Input::ID::RESET) trip.resetTrip();
    else if (id == Input::ID::PAUSE) trip.pause();
  }

  void fillFrame(Frame &frame, const Trip &trip, const Clock &) const override {
    strcpy(frame.header.modeSpeed, "SPD");
    strcpy(frame.header.modeTime, "Time");
    Formatter::formatSpeed(trip.getSpeed(), frame.main.value, sizeof(frame.main.value));
    strcpy(frame.main.unit, "km/h");
    Formatter::formatDuration(trip.getElapsedTimeMs(), frame.sub.value, sizeof(frame.sub.value));
    strcpy(frame.sub.unit, "");
  }
};

class AvgOdoState : public ModeState {
public:
  void onInput(Input::ID id, Trip &trip, DataStore &) override {
    if (id == Input::ID::RESET) trip.reset();
    else if (id == Input::ID::PAUSE) trip.pause();
  }

  void fillFrame(Frame &frame, const Trip &trip, const Clock &) const override {
    strcpy(frame.header.modeSpeed, "AVG");
    strcpy(frame.header.modeTime, "Odo");
    Formatter::formatSpeed(trip.getAvgSpeed(), frame.main.value, sizeof(frame.main.value));
    strcpy(frame.main.unit, "km/h");
    Formatter::formatDistance(trip.getTotalDistance(), frame.sub.value, sizeof(frame.sub.value));
    strcpy(frame.sub.unit, "km");
  }
};

class MaxClockState : public ModeState {
public:
  void onInput(Input::ID id, Trip &trip, DataStore &) override {
    if (id == Input::ID::RESET) trip.resetMaxSpeed();
    else if (id == Input::ID::PAUSE) trip.pause();
  }

  void fillFrame(Frame &frame, const Trip &trip, const Clock &clock) const override {
    strcpy(frame.header.modeSpeed, "MAX");
    strcpy(frame.header.modeTime, "Clock");
    Formatter::formatSpeed(trip.getMaxSpeed(), frame.main.value, sizeof(frame.main.value));
    strcpy(frame.main.unit, "km/h");
    Formatter::formatTime(clock.getTime(), frame.sub.value, sizeof(frame.sub.value));
    strcpy(frame.sub.unit, "");
  }
};

class Mode {
private:
  SpdTimeState  spdTimeState;
  AvgOdoState   avgOdoState;
  MaxClockState maxClockState;

  ModeState *currentState;

  ModeState *states[3];
  int        currentIndex = 0;

public:
  Mode() : currentState(&spdTimeState) {
    states[0] = &spdTimeState;
    states[1] = &avgOdoState;
    states[2] = &maxClockState;
  }

  void next() {
    currentIndex++;
    if (currentIndex >= 3) currentIndex = 0;
    currentState = states[currentIndex];
  }

  ModeState *getCurrentState() const {
    return currentState;
  }

  void handleInput(Input::ID id, Trip &trip, DataStore &dataStore) {
    if (id == Input::ID::RESET_LONG) {
      trip.reset();
      dataStore.clear();
      return;
    }

    if (id == Input::ID::SELECT) {
      next();
      return;
    }
    currentState->onInput(id, trip, dataStore);
  }
};
