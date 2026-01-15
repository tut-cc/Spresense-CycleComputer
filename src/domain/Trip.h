#pragma once

#include <GNSS.h>

#include "Odometer.h"
#include "Speedometer.h"
#include "Stopwatch.h"

class Trip {
public:
  Speedometer speedometer;
  Odometer    odometer;
  Stopwatch   stopwatch;

private:
  unsigned long lastMillis;
  bool          hasLastMillis;

public:
  void begin() {
    reset();
  }

  void update(const SpNavData &navData, unsigned long currentMillis) {
    const float rawKmh = navData.velocity * 60.0f * 60.0f / 1000.0f;
    const float filteredKmh = rawKmh * 0.2f + speedometer.getCur() * 0.8f; // 簡易移動平均フィルタ
    const bool hasFix = navData.posFixMode != FixInvalid;
    const bool isMoving = hasFix && (Config::MIN_MOVING_SPEED_KMH < filteredKmh); // GPS ノイズ対策
    const float speedKmh = isMoving ? filteredKmh : 0.0f;

    if (!hasLastMillis) {
      lastMillis    = currentMillis;
      hasLastMillis = true;
      return;
    }

    const unsigned long dt = currentMillis - lastMillis;
    lastMillis             = currentMillis;

    stopwatch.update(isMoving, dt);
    if (hasFix) odometer.update(navData.latitude, navData.longitude, isMoving);
    speedometer.update(speedKmh, stopwatch.getMovingTimeMs(), odometer.getTotalDistance());
  }

  void reset() {
    speedometer.reset();
    odometer.reset();
    stopwatch.reset();
    lastMillis    = 0;
    hasLastMillis = false;
  }
};
