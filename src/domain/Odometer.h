#pragma once

#include <math.h>

#include "../Config.h"

class Odometer {
private:
  float totalKm      = 0.0f;
  float lastLat      = 0.0f;
  float lastLon      = 0.0f;
  bool  hasLastCoord = false;

public:
  void update(float lat, float lon, bool isMoving) {
    if (fabsf(lat) < Config::Odometer::MIN_ABS && fabsf(lon) < Config::Odometer::MIN_ABS) {
      return; // 無効な値を避ける
    }

    if (!hasLastCoord) {
      lastLat      = lat;
      lastLon      = lon;
      hasLastCoord = true;
      return;
    }

    if (isMoving) {
      const float deltaKm = planarDistanceKm(lastLat, lastLon, lat, lon);
      if (Config::Odometer::MIN_DELTA < deltaKm &&
          deltaKm < Config::Odometer::MAX_DELTA) { // GPS ノイズ対策
        totalKm += deltaKm;
      }
    }

    lastLat = lat;
    lastLon = lon;
  }

  void reset() {
    totalKm      = 0.0f;
    lastLat      = 0.0f;
    lastLon      = 0.0f;
    hasLastCoord = false;
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
