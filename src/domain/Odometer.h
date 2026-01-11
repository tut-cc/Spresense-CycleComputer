#pragma once

#include <GNSS.h>

/**
 * @brief Logic for tracking total distance.
 *
 * Design Decision:
 * - Pure Logic: Receives pre-calculated distance deltas (km) rather than raw GPS data.
 *   The calculation of "distance from speed/coordinates" is delegated to the caller (Trip),
 *   allowing Odometer to focus solely on accumulation and persistence (future).
 */
class Odometer {
private:
  float distanceKm;

public:
  Odometer() : distanceKm(0.0f) {}

  void update(float distanceDeltaKm) {
    distanceKm += distanceDeltaKm;
  }

  void reset() {
    distanceKm = 0.0f;
  }

  float getDistance() const {
    return distanceKm;
  }
};
