#pragma once

#include "Odometer.h"
#include "Speedometer.h"
#include "Stopwatch.h"

namespace application {

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

  void update(float currentSpeedKmh, unsigned long currentMillis) {
    if (lastUpdateTime == 0) {
      lastUpdateTime = currentMillis;
      stopwatch.update(false, 0, currentMillis); // Initialize start time logic if needed
      return;
    }

    unsigned long dt = currentMillis - lastUpdateTime;
    lastUpdateTime   = currentMillis;

    speedometer.update(currentSpeedKmh);

    bool isMoving = (currentSpeedKmh > 3.0f);
    odometer.update(currentSpeedKmh, dt);
    stopwatch.update(isMoving, dt, currentMillis);
  }

  void reset() {
    speedometer.reset();
    odometer.reset();
    stopwatch.reset();
    lastUpdateTime = 0;
  }

  // Getters delegating to components
  float getMaxSpeedKmh() const {
    return speedometer.getMax();
  }

  float getDistanceKm() const {
    return odometer.getDistance();
  }

  float getAvgSpeedKmh() const {
    unsigned long movingTimeMs = stopwatch.getMovingTime();
    if (movingTimeMs == 0) return 0.0f;
    return (odometer.getDistance() / (movingTimeMs / 3600000.0f));
  }

  void getMovingTimeStr(char *buffer, size_t size) const {
    stopwatch.getMovingTimeStr(buffer, size);
  }

  void getElapsedTimeStr(char *buffer, size_t size) const {
    stopwatch.getElapsedTimeStr(buffer, size);
  }
};

} // namespace application
