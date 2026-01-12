#pragma once

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
