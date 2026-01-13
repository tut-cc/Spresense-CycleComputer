#pragma once

#include <cmath>

#ifndef PI
#define PI 3.1415926535897932384626433832795
#endif

class Odometer {
public:
  void update(double lat, double lon, bool accumulate) {
    if (!initialized) {
      lastLat     = lat;
      lastLon     = lon;
      initialized = true;
      return;
    }

    if (accumulate) {
      float distanceDeltaKm = calculateDistanceKm(lastLat, lastLon, lat, lon);
      distance.totalKm += distanceDeltaKm;
    }

    lastLat = lat;
    lastLon = lon;
  }

  void reset() {
    distance    = {};
    initialized = false;
    lastLat     = 0.0;
    lastLon     = 0.0;
  }

  float getDistance() const {
    return distance.totalKm;
  }

private:
  struct Distance {
    float totalKm = 0.0f;
  };

  Distance distance;
  bool     initialized = false;
  double   lastLat     = 0.0;
  double   lastLon     = 0.0;

  double toRadians(double degrees) {
    return degrees * PI / 180.0;
  }

  float calculateDistanceKm(double lat1, double lon1, double lat2, double lon2) {
    double R    = 6371.0; // Earth radius in km
    double dLat = toRadians(lat2 - lat1);
    double dLon = toRadians(lon2 - lon1);
    double a =
        std::sin(dLat / 2) * std::sin(dLat / 2) + std::cos(toRadians(lat1)) * std::cos(toRadians(lat2)) * std::sin(dLon / 2) * std::sin(dLon / 2);
    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
    return (float)(R * c);
  }
};
