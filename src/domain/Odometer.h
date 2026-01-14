#pragma once

#include <math.h>

#ifndef PI
#define PI 3.1415926535897932384626433832795f
#endif

class Odometer {
private:
  float totalKm     = 0.0f;
  float lastLat     = 0.0f;
  float lastLon     = 0.0f;
  bool  initialized = false;

public:
  void update(float lat, float lon, bool isMoving) {
    if (fabsf(lat) < 1e-6f && fabsf(lon) < 1e-6f) return; // 無効な値を避ける

    if (!initialized) {
      lastLat     = lat;
      lastLon     = lon;
      initialized = true;
      return;
    }

    if (isMoving) {
      const float deltaKm = planarDistanceKm(lastLat, lastLon, lat, lon);
      if (0.001f < deltaKm && deltaKm < 1.0f) { // GPS ノイズ対策
        totalKm += deltaKm;
      }
    }

    lastLat = lat;
    lastLon = lon;
  }

  void reset() {
    totalKm     = 0.0f;
    lastLat     = 0.0f;
    lastLon     = 0.0f;
    initialized = false;
  }

  float getTotalDistance() const {
    return totalKm;
  }

private:
  static constexpr float toRadians(float degrees) {
    return degrees * PI / 180.0f;
  }

  static float planarDistanceKm(float lat1, float lon1, float lat2, float lon2) {
    constexpr float R      = 6378137.0f; // WGS84 [m]
    const float     latRad = toRadians((lat1 + lat2) / 2.0f);
    const float     dLat   = toRadians(lat2 - lat1);
    const float     dLon   = toRadians(lon2 - lon1);
    const float     x      = dLon * cosf(latRad) * R;
    const float     y      = dLat * R;
    return sqrtf(x * x + y * y) / 1000.0f; // km
  }
};
