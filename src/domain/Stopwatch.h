#pragma once

#include <Arduino.h>
#include <GNSS.h>


class Stopwatch {
private:
  unsigned long movingTimeMs;
  unsigned long startTimeMs;

public:
  Stopwatch() : movingTimeMs(0), startTimeMs(0) {}

  void update(const SpNavData &navData, unsigned long dtMs, unsigned long currentMillis) {
    if (startTimeMs == 0) startTimeMs = currentMillis;

    float speedKmh = navData.velocity * 3.6f;
    if (navData.posFixMode == FixInvalid) speedKmh = 0.0f;
    bool isMoving = (speedKmh > 3.0f);

    if (isMoving) movingTimeMs += dtMs;
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

