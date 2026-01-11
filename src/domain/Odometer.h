#pragma once

#include <GNSS.h>


class Odometer {
private:
  float distanceKm;

public:
  Odometer() : distanceKm(0.0f) {}

  void update(const SpNavData &navData, unsigned long dtMs) {
    float currentSpeedKmh = navData.velocity * 3.6f;
    if (navData.posFixMode == FixInvalid) currentSpeedKmh = 0.0f;
    if (navData.velocity > 1.0f) {
      if (currentSpeedKmh > 3.0f) {
        float distanceInc = (currentSpeedKmh * dtMs) / 3600000.0f;
        distanceKm += distanceInc;
      }
    }
  }

  void reset() {
    distanceKm = 0.0f;
  }

  float getDistance() const {
    return distanceKm;
  }
};

