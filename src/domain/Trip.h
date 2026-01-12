#pragma once

#include "Odometer.h"
#include "Speedometer.h"
#include "Stopwatch.h"

#include <GNSS.h>

/**
 * @brief Manage trip related data (Speed, Distance, Time).
 *
 * Design Decision:
 * 1. Data Distribution:
 *    Trip class acts as a "Distributor" of the SpNavData. It extracts refined values
 *    (e.g. speed in km/h, distance delta) from the huge SpNavData structure and
 *    passes them to the child components (Speedometer, Odometer, Stopwatch).
 *    This prevents child components from depending on the hardware-specific SpNavData.
 *
 * 2. Public Members:
 *    Member objects (speedometer, odometer, stopwatch) are public to avoid redundant
 *    pass-through getters and to keep the class simple as a data holder.
 *    Access them directly like `trip.speedometer.get()`.
 */
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
    bool  isMoving = (speedKmh > 2.0f);

    speedometer.update(speedKmh);

    if (isMoving) {
      // Calculate distance increment based on speed and time
      // m/s * ms / 1000 = meters; / 1000 = km
      // (navData.velocity * dt) / 1000000.0f
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
