#pragma once

#include <Arduino.h>
#include <math.h>

class Odometer {
private:
  float totalKm      = 0.0f;
  float lastLat      = 0.0f;
  float lastLon      = 0.0f;
  bool  hasLastCoord = false;

  static bool isValidCoordinate(float lat, float lon) {
    return !(fabsf(lat) < MIN_ABS && fabsf(lon) < MIN_ABS);
  }

private:
  static constexpr float MIN_ABS   = 1e-6f;
  static constexpr float MIN_DELTA = 0.002f;
  static constexpr float MAX_DELTA = 1.0f;

public:
  float update(float lat, float lon, bool isMoving) {
    if (!isValidCoordinate(lat, lon)) {
      return 0.0f; // Avoid invalid values
    }

    if (!hasLastCoord) {
      lastLat      = lat;
      lastLon      = lon;
      hasLastCoord = true;
      return 0.0f;
    }

    float deltaKm = 0.0f;
    if (isMoving) {
      const float dist         = planarDistanceKm(lastLat, lastLon, lat, lon);
      const bool  isDeltaValid = MIN_DELTA < dist && dist < MAX_DELTA;
      if (isDeltaValid) {
        deltaKm = dist;
        totalKm += deltaKm; // Anti-GPS noise
      }
    }

    lastLat = lat;
    lastLon = lon;

    return deltaKm;
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

  void setTotalDistance(float dist) {
    totalKm = dist;
  }

private:
  static constexpr float toRad(float degrees) {
    return degrees * PI / 180.0f;
  }

  static float planarDistanceKm(float lat1, float lon1, float lat2, float lon2) {
    constexpr float R      = 6378137.0f; // WGS84 [m]
    const float     latRad = toRad((lat1 + lat2) / 2.0f);
    const float     dLat   = toRad(lat2 - lat1);
    const float     dLon   = toRad(lon2 - lon1);
    const float     x      = dLon * cosf(latRad) * R;
    const float     y      = dLat * R;
    return sqrtf(x * x + y * y) / 1000.0f; // km
  }
};
