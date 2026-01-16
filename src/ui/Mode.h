#pragma once

#include "../domain/Clock.h"
#include "../domain/DataStore.h"
#include "../domain/Trip.h"
#include "Input.h"
#include "Renderer.h"
#include <Arduino.h>

class Mode {
public:
  enum class ID { SPD_TIME, AVG_ODO, MAX_CLOCK };

private:
  ID currentMode = ID::SPD_TIME;

public:
  void handleInput(Input::ID id, Trip &trip, DataStore &dataStore) {
    if (id == Input::ID::RESET_LONG) {
      trip.reset();
      dataStore.clear();
      return;
    }

    if (id == Input::ID::SELECT) {
      currentMode = static_cast<ID>((static_cast<int>(currentMode) + 1) % 3);
      return;
    }

    if (id == Input::ID::PAUSE) {
      trip.pause();
      return;
    }

    if (id == Input::ID::RESET) {
      switch (currentMode) {
      case ID::SPD_TIME:
        trip.resetTrip();
        return;
      case ID::AVG_ODO:
        trip.reset();
        return;
      case ID::MAX_CLOCK:
        trip.resetMaxSpeed();
        return;
      }
    }
  }

  void fillFrame(Frame &frame, const Trip &trip, const Clock &clock) const {
    const Trip::State &state = trip.getState();

    // Default units
    strcpy(frame.main.unit, "km/h");
    strcpy(frame.sub.unit, "");

    switch (currentMode) {
    case ID::SPD_TIME:
      strcpy(frame.header.modeSpeed, "SPD");
      Formatter::formatSpeed(state.currentSpeed, frame.main.value, sizeof(frame.main.value));

      strcpy(frame.header.modeTime, "Time");
      if (state.isPaused && (millis() / 500) % 2 == 0) strcpy(frame.sub.value, "");
      else
        Formatter::formatDuration(state.totalElapsedMs, frame.sub.value, sizeof(frame.sub.value));
      break;

    case ID::AVG_ODO:
      strcpy(frame.header.modeSpeed, "AVG");
      strcpy(frame.header.modeTime, "Odo");
      Formatter::formatSpeed(state.avgSpeed, frame.main.value, sizeof(frame.main.value));
      Formatter::formatDistance(state.totalKm, frame.sub.value, sizeof(frame.sub.value));
      strcpy(frame.sub.unit, "km");
      break;

    case ID::MAX_CLOCK:
      strcpy(frame.header.modeSpeed, "MAX");
      strcpy(frame.header.modeTime, "Clock");
      Formatter::formatSpeed(state.maxSpeed, frame.main.value, sizeof(frame.main.value));
      Formatter::formatTime(clock.getTime(), frame.sub.value, sizeof(frame.sub.value));
      break;
    }
  }
};
