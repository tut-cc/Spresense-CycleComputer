#pragma once

#include "../logic/Clock.h"
#include "../logic/DataStore.h"
#include "../logic/Trip.h"
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
    currentMode = calculateNextMode(currentMode, id);

    switch (id) {
    case Input::Event::RESET_LONG:
      trip.reset();
      dataStore.clear();
      break;
    case Input::Event::PAUSE:
      trip.pause();
      break;
    case Input::Event::RESET:
      handleReset(trip);
      break;
    default:
      break;
    }
  }

  void fillFrame(Frame &frame, const Trip &trip, const Clock &clock) const {
    const bool blinkVisible = (millis() / 500) % 2 == 0;
    updateFrame(frame, currentMode, trip.getState(), clock, blinkVisible);
  }

private:
  static ID calculateNextMode(ID current, Input::Event event) {
    if (event == Input::Event::SELECT) {
      return static_cast<ID>((static_cast<int>(current) + 1) % 3);
    }
    return current;
  }

  void handleReset(Trip &trip) {
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
  }

  static void renderSpdTim(Frame &frame, const Trip::State &state, bool blinkVisible) {
    strcpy(frame.header.modeSpeed, "SPD");
    strcpy(frame.header.modeTime, "Time");
    Formatter::formatSpeed(state.currentSpeed, frame.main.value, sizeof(frame.main.value));

    if (state.status == Trip::Status::Paused && !blinkVisible) {
      strcpy(frame.sub.value, "");
    } else {
      Formatter::formatDuration(state.totalElapsedMs, frame.sub.value, sizeof(frame.sub.value));
    }

    strcpy(frame.main.unit, "km/h");
    strcpy(frame.sub.unit, "");
  }

  static void renderAvgOdo(Frame &frame, const Trip::State &state) {
    strcpy(frame.header.modeSpeed, "AVG");
    strcpy(frame.header.modeTime, "Odo");
    Formatter::formatSpeed(state.avgSpeed, frame.main.value, sizeof(frame.main.value));
    Formatter::formatDistance(state.totalKm, frame.sub.value, sizeof(frame.sub.value));
    strcpy(frame.main.unit, "km/h");
    strcpy(frame.sub.unit, "km");
  }

  static void renderMaxClk(Frame &frame, const Trip::State &state, const Clock &clock) {
    strcpy(frame.header.modeSpeed, "MAX");
    strcpy(frame.header.modeTime, "Clock");
    Formatter::formatSpeed(state.maxSpeed, frame.main.value, sizeof(frame.main.value));
    Formatter::formatTime(clock, frame.sub.value, sizeof(frame.sub.value));
    strcpy(frame.main.unit, "km/h");
    strcpy(frame.sub.unit, "");
  }

  static void updateFrame(Frame &frame, ID mode, const Trip::State &state, const Clock &clock,
                          bool blinkVisible) {
    switch (mode) {
    case ID::SPD_TIM:
      renderSpdTim(frame, state, blinkVisible);
      break;
    case ID::AVG_ODO:
      renderAvgOdo(frame, state);
      break;
    case ID::MAX_CLK:
      renderMaxClk(frame, state, clock);
      break;
    }
  }
};
