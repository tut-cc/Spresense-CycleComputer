#pragma once

#include <math.h>

#ifndef PI
#define PI 3.1415926535897932384626433832795f
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
  void update(float lat, float lon, bool isMoving) {
    if (abs(lat) < 1e-6f && abs(lon) < 1e-6f) return; // 無効な値を避ける

    if (!initialized) {
      lastLat     = lat;
      lastLon     = lon;
      initialized = true;
      return;
    }

    if (isMoving) {
      const float distanceDeltaKm = calculateDistanceKm(lastLat, lastLon, lat, lon);
      if (0.001f < distanceDeltaKm && distanceDeltaKm < 1.0f) { // GPS ノイズ対策
        distance.totalKm += distanceDeltaKm;
      }
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
  float toRadians(float degrees) const {
    return degrees * PI / 180.0f;
  }

  float calculateDistanceKm(float lat1, float lon1, float lat2, float lon2) const {
    return gaussMidLatitude(lat1, lon1, lat2, lon2);
  }

  float Haversine(float lat1, float lon1, float lat2, float lon2) const {
    constexpr float R       = 6371.0f; // 地球の半径 (km)
    const float     dLat    = toRadians((lat2 - lat1) / 2.0f);
    const float     dLon    = toRadians((lon2 - lon1) / 2.0f);
    const float     sinDLat = sinf(dLat);
    const float     sinDLon = sinf(dLon);
    const float     cosLat1 = cosf(toRadians(lat1));
    const float     cosLat2 = cosf(toRadians(lat2));
    const float     a       = sinDLat * sinDLat + cosLat1 * cosLat2 * sinDLon * sinDLon;
    const float     c       = 2.0f * atan2f(sqrtf(a), sqrtf(1.0f - a));
    return R * c;
  }

  float gaussMidLatitude(float lat1, float lon1, float lat2, float lon2) const {
    constexpr float a           = 6378137.0f;          // GRS80
    constexpr float f           = 1.0 / 298.257223563; // WGS84
    constexpr float e2          = f * (2.0f - f);
    const float     latdiffhalf = toRadians((lat1 - lat2) / 2.0f);
    const float     londiffhalf = toRadians((lon1 - lon2) / 2.0f);
    const float     latmid      = toRadians((lat1 + lat2) / 2.0f);
    const float     sinlat      = sinf(latmid);
    const float     coslat      = cosf(latmid);
    const float     n2          = 1.0f / (1.0f - e2 * sinlat * sinlat);
    const float     n           = sqrtf(n2);        // prime vertical radius of curvature
    const float     m_by_n      = (1.0f - e2) * n2; // meridian ratio
    const float     x           = sinf(londiffhalf) * coslat;
    const float     y           = cosf(londiffhalf) * sinf(latdiffhalf * m_by_n);
    auto            asin2       = [](float x) { return x < 1.0f ? 2.0f * asinf(x) : PI; };
    return n * a * asin2(hypotf(x, y));
  }
};
