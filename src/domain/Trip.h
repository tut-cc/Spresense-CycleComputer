#pragma once

#include <GNSS.h>

#include "Odometer.h"
#include "Speedometer.h"
#include "Stopwatch.h"

class Trip {
public:
  Speedometer speedometer;
  Odometer    odometer;
  Stopwatch   stopwatch;

private:
  unsigned long lastMillis;
  bool          initialized;

public:
  Trip() : lastMillis(0), initialized(false) {}

  void begin() {
    reset();
  }

  void update(const SpNavData &navData, unsigned long currentMillis) {
    const float speedKmh = navData.velocity * 3.6f;
    const bool  isMoving = (0.5f < speedKmh); // GPS ノイズ対策

    if (!initialized) {
      lastMillis  = currentMillis;
      initialized = true;
      return;
    }

    const unsigned long dt = currentMillis - lastMillis;
    lastMillis             = currentMillis;

    stopwatch.update(isMoving, dt);
    odometer.update(navData.latitude, navData.longitude, isMoving);
    speedometer.update(speedKmh, stopwatch.getMovingTimeMs(), odometer.getDistance());
  }

  void reset() {
    speedometer.reset();
    odometer.reset();
    stopwatch.reset();
    lastMillis  = 0;
    initialized = false;
  }
};
