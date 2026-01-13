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
  unsigned long lastUpdateTime;
  bool          initialized;

public:
  Trip() : lastUpdateTime(0), initialized(false) {}

  void begin() {
    reset();
  }

  void update(const SpNavData &navData, unsigned long currentMillis) {
    float speedKmh = navData.velocity * 3.6f;
    bool  isMoving = (speedKmh > 2.0f);

    if (!initialized) {
      lastUpdateTime = currentMillis;
      initialized    = true;
      return;
    }

    unsigned long dt = currentMillis - lastUpdateTime;
    lastUpdateTime   = currentMillis;

    speedometer.update(speedKmh);
    stopwatch.update(isMoving, dt);
    odometer.update(navData.latitude, navData.longitude, isMoving);
  }

  void reset() {
    speedometer.reset();
    odometer.reset();
    stopwatch.reset();
    lastUpdateTime = 0;
    initialized    = false;
  }

  float getAvgSpeedKmh() const {
    unsigned long movingTimeMs = stopwatch.getMovingTimeMs();
    if (movingTimeMs == 0) return 0.0f;
    return (odometer.getDistance() / (movingTimeMs / 3600000.0f));
  }
};
