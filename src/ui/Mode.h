#pragma once

#include "../domain/DataStore.h"
#include "../domain/Trip.h"

#include "ConcreteModes.h"

class Mode {
private:
  SpdTimeState  spdTimeState;
  AvgOdoState   avgOdoState;
  MaxClockState maxClockState;

  ModeState *currentState;

  // We use an internal index just for next() logic simplicity, or a circular linked list approach.
  // Simple approach: Array of pointers
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

  void reset(Trip &trip, DataStore &dataStore) {
    currentState->reset(trip, dataStore);
  }
};
