#pragma once

#include <cstring>

#include "../domain/Clock.h"
#include "../domain/Trip.h"

#include "Frame.h"
#include "ModeState.h"

class FrameBuilder {
public:
  static Frame build(const Trip &trip, const Clock &clock, const ModeState &modeState,
                     FixType fixType) {

    Frame frame;

    // Set Header Fix Status
    switch (fixType) {
    case FixType::NoFix:
      strcpy(frame.header.fixStatus, "WAIT");
      break;
    case FixType::Fix2D:
      strcpy(frame.header.fixStatus, "2D");
      break;
    case FixType::Fix3D:
      strcpy(frame.header.fixStatus, "3D");
      break;
    default:
      strcpy(frame.header.fixStatus, "");
      break;
    }

    modeState.fillFrame(frame, trip, clock);

    return frame;
  }
};
