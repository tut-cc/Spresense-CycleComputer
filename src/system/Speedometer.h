#pragma once

#include <GNSS.h>

namespace application {

class Speedometer {
private:
  float speedKmh;
  float maxSpeedKmh;

public:
  Speedometer() : speedKmh(0.0f), maxSpeedKmh(0.0f) {}

  void update(const SpNavData &navData) {
    speedKmh = navData.velocity * 3.6f;
    if (navData.posFixMode == FixInvalid) speedKmh = 0.0f;
    if (speedKmh > maxSpeedKmh) maxSpeedKmh = speedKmh;
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

} // namespace application
