#pragma once

#include "../domain/Clock.h"
#include "../domain/DataStore.h"
#include "../domain/Trip.h"
#include "Input.h"
#include "Renderer.h"
#include <Arduino.h>

class Mode {
public:
  enum class ID { SPD_TIM, AVG_ODO, MAX_CLK };

private:
  ID currentMode = ID::SPD_TIM;

public:
  void handleInput(Input::Event id, Trip &trip, DataStore &dataStore) {
    switch (id) {
    case Input::Event::RESET_LONG:
      trip.reset();
      dataStore.clear();
      break;

    case Input::Event::SELECT:
      currentMode = static_cast<ID>((static_cast<int>(currentMode) + 1) % 3);
      break;

    case Input::Event::PAUSE:
      trip.pause();
      break;

    case Input::Event::RESET:
      switch (currentMode) {
      case ID::SPD_TIM:
        trip.resetTrip();
        break;
      case ID::AVG_ODO:
        trip.reset();
        break;
      case ID::MAX_CLK:
        trip.resetMaxSpeed();
        break;
      }
      break;

    case Input::Event::NONE:
      break;
    }
  }

  void fillFrame(Frame &frame, const Trip &trip, const Clock &clock) const {
    const Trip::State &state = trip.getState();

    strcpy(frame.main.unit, "km/h");
    strcpy(frame.sub.unit, "");

    switch (currentMode) {
    case ID::SPD_TIM:
      strcpy(frame.header.modeSpeed, "SPD");
      Formatter::formatSpeed(state.currentSpeed, frame.main.value, sizeof(frame.main.value));

      strcpy(frame.header.modeTime, "Time");
      if (state.status == Trip::Status::Paused && (millis() / 500) % 2 == 0) {
        strcpy(frame.sub.value, "");
      } else {
        Formatter::formatDuration(state.totalElapsedMs, frame.sub.value, sizeof(frame.sub.value));
      }
      break;

    case ID::AVG_ODO:
      strcpy(frame.header.modeSpeed, "AVG");
      strcpy(frame.header.modeTime, "Odo");
      Formatter::formatSpeed(state.avgSpeed, frame.main.value, sizeof(frame.main.value));
      Formatter::formatDistance(state.totalKm, frame.sub.value, sizeof(frame.sub.value));
      strcpy(frame.sub.unit, "km");
      break;

    case ID::MAX_CLK:
      strcpy(frame.header.modeSpeed, "MAX");
      strcpy(frame.header.modeTime, "Clock");
      Formatter::formatSpeed(state.maxSpeed, frame.main.value, sizeof(frame.main.value));
      Formatter::formatTime(clock, frame.sub.value, sizeof(frame.sub.value));
      break;
    }
  }
};
