#pragma once

#include <cmath>

#ifndef PI
#define PI 3.1415926535897932384626433832795
#endif

class Odometer {
private:
  struct Distance {
    float totalKm = 0.0f;
  };

  Distance distance;
  bool     initialized = false;
  float    lastLat     = 0.0f;
  float    lastLon     = 0.0f;

public:
  void update(const float lat, const float lon, const bool accumulate) {
    if (std::abs(lat) < 1e-6 && std::abs(lon) < 1e-6) return;

    if (!initialized) {
      lastLat     = lat;
      lastLon     = lon;
      initialized = true;
      return;
    }

    if (accumulate) {
      const float distanceDeltaKm = calculateDistanceKm(lastLat, lastLon, lat, lon);
      if (distanceDeltaKm < 1.0f) distance.totalKm += distanceDeltaKm;
    }

    lastLat = lat;
    lastLon = lon;
  }

  void reset() {
    distance    = {};
    initialized = false;
    lastLat     = 0.0f;
    lastLon     = 0.0f;
  }

  float getDistance() const {
    return distance.totalKm;
  }

private:
  float toRadians(const float degrees) const {
    return degrees * PI / 180.0f;
  }

  // Haversine の公式
  float calculateDistanceKm(const float lat1, const float lon1, const float lat2, const float lon2) const {
    const float R       = 6371.0f; // 地球の半径 (km)
    const float dLat    = toRadians(lat2 - lat1);
    const float dLon    = toRadians(lon2 - lon1);
    const float sinDLat = std::sin(dLat / 2.0f);
    const float sinDLon = std::sin(dLon / 2.0f);
    const float cosLat1 = std::cos(toRadians(lat1));
    const float cosLat2 = std::cos(toRadians(lat2));
    const float a       = sinDLat * sinDLat + cosLat1 * cosLat2 * sinDLon * sinDLon;
    const float c       = 2.0f * std::atan2(std::sqrt(a), std::sqrt(1.0f - a));
    return R * c;
  }
};
