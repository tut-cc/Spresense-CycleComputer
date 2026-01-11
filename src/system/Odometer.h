#pragma once

namespace application {

class Odometer {
private:
  float distanceKm;

public:
  Odometer() : distanceKm(0.0f) {}

  void update(float currentSpeedKmh, unsigned long dtMs) {
    if (currentSpeedKmh > 3.0f) { // Moving threshold
      float distanceInc = (currentSpeedKmh * dtMs) / 3600000.0f;
      distanceKm += distanceInc;
    }
  }

  void reset() {
    distanceKm = 0.0f;
  }

  float getDistance() const {
    return distanceKm;
  }
};

} // namespace application
