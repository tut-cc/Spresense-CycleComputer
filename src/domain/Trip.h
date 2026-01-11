#pragma once

#include "Odometer.h"
#include "Speedometer.h"
#include "Stopwatch.h"

#include <GNSS.h>


class Trip {
private:
  Speedometer   speedometer;
  Odometer      odometer;
  Stopwatch     stopwatch;
  unsigned long lastUpdateTime;

public:
  Trip() : lastUpdateTime(0) {}

  void begin() {
    reset();
  }

  void update(const SpNavData &navData, unsigned long currentMillis) {
    speedometer.update(navData);

    if (lastUpdateTime == 0) {
      lastUpdateTime = currentMillis;
      stopwatch.update(navData, 0, currentMillis); // Initialize start time logic if needed
      return;
    }

    unsigned long dt = currentMillis - lastUpdateTime;
    lastUpdateTime   = currentMillis;

    odometer.update(navData, dt);
    stopwatch.update(navData, dt, currentMillis);
  }

  void reset() {
    speedometer.reset();
    odometer.reset();
    stopwatch.reset();
    lastUpdateTime = 0;
  }

  float getSpeedKmh() const {
    return speedometer.get();
  }

  float getMaxSpeedKmh() const {
    return speedometer.getMax();
  }

  float getAvgSpeedKmh() const {
    unsigned long movingTimeMs = stopwatch.getMovingTimeMs();
    if (movingTimeMs == 0) return 0.0f;
    return (odometer.getDistance() / (movingTimeMs / 3600000.0f));
  }

  float getDistanceKm() const {
    return odometer.getDistance();
  }

  unsigned long getMovingTimeMs() const {
    return stopwatch.getMovingTimeMs();
  }

  unsigned long getElapsedTimeMs() const {
    return stopwatch.getElapsedTimeMs();
  }
};

