#pragma once

#include "Odometer.h"
#include "Speedometer.h"
#include "Stopwatch.h"

#include <Arduino.h>
#include <GNSS.h>

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
    if (!initialized) {
      lastUpdateTime = currentMillis;
      initialized    = true;
      return;
    }

    unsigned long dt = currentMillis - lastUpdateTime;
    lastUpdateTime   = currentMillis;

    float speedKmh = navData.velocity * 3.6f;

    bool isMoving = (speedKmh > 2.0f);

    speedometer.update(speedKmh);

    if (isMoving) {
      float distanceDeltaKm = (navData.velocity * dt) / 1000000.0f;
      odometer.update(distanceDeltaKm);
    } else {
      odometer.update(0.0f);
    }

    stopwatch.update(isMoving, dt);
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
