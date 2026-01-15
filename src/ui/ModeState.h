#pragma once

#include "../domain/Clock.h"
#include "../domain/DataStore.h"
#include "../domain/Trip.h"
#include "Frame.h"

class ModeState {
public:
  virtual ~ModeState() = default;

  virtual void reset(Trip &trip, DataStore &dataStore)                             = 0;
  virtual void fillFrame(Frame &frame, const Trip &trip, const Clock &clock) const = 0;
};
