#pragma once

#include <GNSS.h>

/**
 * @brief Logic for tracking current and maximum speed.
 *
 * Design Decision:
 * - Pure Logic: This class accepts simple `float` values (speed in km/h) instead of
 *   the raw `SpNavData` structure. This makes the class testable without hardware
 *   dependencies and reusable in other contexts.
 */
class Speedometer {
private:
  float speedKmh;
  float maxSpeedKmh;

public:
  Speedometer() : speedKmh(0.0f), maxSpeedKmh(0.0f) {}

  void update(float speedKmh) {
    this->speedKmh = speedKmh;
    if (this->speedKmh > maxSpeedKmh) maxSpeedKmh = this->speedKmh;
  }

  void reset() {
    speedKmh    = 0.0f;
    maxSpeedKmh = 0.0f;
  }

  float get() const {
    return speedKmh;
  }

  float getMax() const {
    return maxSpeedKmh;
  }
};
