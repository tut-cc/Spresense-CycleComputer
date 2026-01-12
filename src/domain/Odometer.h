#pragma once

/**
 * @brief Logic for tracking total distance.
 *
 * Design Decision:
 * - Pure Logic: Receives pre-calculated distance deltas (km) rather than raw GPS data.
 *   The calculation of "distance from speed/coordinates" is delegated to the caller (Trip),
 *   allowing Odometer to focus solely on accumulation and persistence (future).
 */
class Odometer {
public:
  void update(float distanceDeltaKm) {
    distance.totalKm += distanceDeltaKm;
  }

  void reset() {
    distance = {};
  }

  float getDistance() const {
    return distance.totalKm;
  }

private:
  struct Distance {
    float totalKm = 0.0f;
  };

  Distance distance;
};
