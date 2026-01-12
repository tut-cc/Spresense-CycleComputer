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
public:
  void update(float currentKmh) {
    speed.currentKmh = currentKmh;
    if (speed.currentKmh > speed.maxKmh) speed.maxKmh = speed.currentKmh;
  }

  void reset() {
    speed = {};
  }

  float get() const {
    return speed.currentKmh;
  }

  float getMax() const {
    return speed.maxKmh;
  }

private:
  struct Speed {
    float currentKmh = 0.0f;
    float maxKmh     = 0.0f;
  };

  Speed speed;
};
