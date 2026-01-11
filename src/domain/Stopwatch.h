#pragma once

#include <Arduino.h>
#include <GNSS.h>

class Stopwatch {
private:
  unsigned long movingTimeMs;
  unsigned long startTimeMs;
  unsigned long elapsedTimeMs; // Added to track elapsed time directly

public:
  Stopwatch() : movingTimeMs(0), startTimeMs(0), elapsedTimeMs(0) {} // Initialize new member

  void update(bool isMoving, unsigned long dt) {
    if (isMoving) { movingTimeMs += dt; }
    elapsedTimeMs += dt; // Update elapsed time
  }

  void reset() {
    movingTimeMs = 0;
    startTimeMs  = millis();
  }

  unsigned long getMovingTimeMs() const {
    return movingTimeMs;
  }

  unsigned long getElapsedTimeMs() const {
    if (startTimeMs == 0) return 0;
    return millis() - startTimeMs;
  }
};
