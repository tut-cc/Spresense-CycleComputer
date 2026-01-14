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

  float calculateDistanceKm(const float lat1, const float lon1, const float lat2, const float lon2) const {
    return gaussMidLatitude(lat1, lon1, lat2, lon2);
  }

  float Haversine(const float lat1, const float lon1, const float lat2, const float lon2) const {
    constexpr float R       = 6371.0f; // 地球の半径 (km)
    const float     dLat    = toRadians(lat2 - lat1);
    const float     dLon    = toRadians(lon2 - lon1);
    const float     sinDLat = std::sin(dLat / 2.0f);
    const float     sinDLon = std::sin(dLon / 2.0f);
    const float     cosLat1 = std::cos(toRadians(lat1));
    const float     cosLat2 = std::cos(toRadians(lat2));
    const float     a       = sinDLat * sinDLat + cosLat1 * cosLat2 * sinDLon * sinDLon;
    const float     c       = 2.0f * std::atan2(std::sqrt(a), std::sqrt(1.0f - a));
    return R * c;
  }

  float gaussMidLatitude(const float lat1, const float lon1, const float lat2, const float lon2) const {
    constexpr float a           = 6378137;           // GRS80
    constexpr float f           = 1 / 298.257223563; // WGS84
    constexpr float e2          = f * (2 - f);
    constexpr float degree      = PI / 180;
    auto            asin2       = [](const float x) { return x < 1 ? 2 * std::asin(x) : PI; };
    const float     latdiffhalf = (lat1 - lat2) / 2 * degree;
    const float     londiffhalf = (lon1 - lon2) / 2 * degree;
    const float     latmid      = (lat1 + lat2) / 2 * degree;
    const float     sinlat      = std::sin(latmid);
    const float     coslat      = std::cos(latmid);
    const float     n2          = 1 / (1 - e2 * sinlat * sinlat);
    const float     n           = std::sqrt(n2); // prime vertical radius of curvature
    const float     m_by_n      = (1 - e2) * n2; // meridian ratio

    return n * a * asin2(std::hypot(std::sin(londiffhalf) * coslat, std::cos(londiffhalf) * std::sin(latdiffhalf * m_by_n)));
  }
};
