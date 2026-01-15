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

  void setTripDistance(float dist) {
    tripDistance = dist;
  }

  void setMovingTime(unsigned long ms) {
    stopwatch.setMovingTime(ms);
  }

  float getTripDistance() const {
    return tripDistance;
  }

  unsigned long getMovingTimeMs() const {
    return stopwatch.getMovingTimeMs();
  }

private:
  float         tripDistance = 0.0f;
  unsigned long lastMillis;

  bool hasLastMillis;

  static constexpr float MS_TO_KMH            = 3.6f;
  static constexpr float MIN_MOVING_SPEED_KMH = 0.001f;

public:
  void begin() {
    reset();
  }

  void update(const SpNavData &navData, unsigned long currentMillis) {
    if (!hasLastMillis) {
      lastMillis    = currentMillis;
      hasLastMillis = true;
      return;
    }

    const unsigned long dt = currentMillis - lastMillis;
    lastMillis             = currentMillis;

    // Calculate Speed
    const float rawKmh   = navData.velocity * MS_TO_KMH;
    const bool  hasFix   = navData.posFixMode != FixInvalid;
    const bool  isMoving = hasFix && (MIN_MOVING_SPEED_KMH < rawKmh); // Anti-GPS noise
    const float speedKmh = isMoving ? rawKmh : 0.0f;

    // Update Time
    stopwatch.update(isMoving, dt);

    // Update Distance
    float deltaKm = 0.0f;
    if (hasFix) { deltaKm = odometer.update(navData.latitude, navData.longitude, isMoving); }
    tripDistance += deltaKm;

    // Update Speedometer
    speedometer.update(speedKmh, stopwatch.getMovingTimeMs(), tripDistance);
  }

  void resetTime() {
    stopwatch.resetTotalTime();
    tripDistance  = 0.0f;
    lastMillis    = 0;
    hasLastMillis = false;
  }

  void resetOdometerAndMovingTime() {
    odometer.reset();
    tripDistance = 0.0f;
    stopwatch.resetMovingTime();
  }

  void reset() {
    resetTime();
    resetOdometerAndMovingTime();
  }

  void pause() {
    stopwatch.togglePause();
  }
};
