#pragma once

#include "DataStructures.h"
#include <Arduino.h>
#include <GNSS.h>
#include <math.h>

namespace Pipeline {
// Constants
constexpr float         MS_PER_HOUR          = 3600000.0f;
constexpr float         MIN_ABS              = 1e-6f;
constexpr float         MIN_DELTA            = 0.002f;
constexpr float         MAX_DELTA            = 1.0f;
constexpr float         EARTH_RADIUS_M       = 6378137.0f;
constexpr float         MS_TO_KMH            = 3.6f;
constexpr float         MIN_MOVING_SPEED_KMH = 0.5f; // Adjusted from 0.001f for stability
constexpr unsigned long SIGNAL_TIMEOUT_MS    = 3000;

// ========================================
// Pure Helpers
// ========================================

inline float calculateRawKmh(float velocity) {
  return velocity * MS_TO_KMH;
}
inline bool hasFix(SpFixMode mode) {
  return (mode == Fix2D || mode == Fix3D);
}
inline bool isMoving(bool fix, float rawKmh) {
  return fix && (rawKmh > MIN_MOVING_SPEED_KMH);
}

inline TripStateData::Status determineStatus(TripStateData::Status currentStatus, bool moving) {
  if (currentStatus == TripStateData::Status::Paused) return TripStateData::Status::Paused;
  return moving ? TripStateData::Status::Moving : TripStateData::Status::Stopped;
}

inline float calculateCurrentSpeed(TripStateData::Status status, float rawKmh) {
  return (status == TripStateData::Status::Moving) ? rawKmh : 0.0f;
}

inline bool isGnssTimedOut(unsigned long currentMs, unsigned long lastUpdateMs) {
  return (currentMs - lastUpdateMs > SIGNAL_TIMEOUT_MS);
}

inline float calculateAverageSpeed(float tripDistance, unsigned long totalMovingMs) {
  if (totalMovingMs == 0) return 0.0f;
  return tripDistance / (totalMovingMs / MS_PER_HOUR);
}

inline float calculateEffectiveDistance(float dist) {
  return (dist > MIN_DELTA && dist <= MAX_DELTA) ? dist : 0.0f;
}

inline bool isValidCoordinate(float lat, float lon) {
  return !(fabsf(lat) < MIN_ABS && fabsf(lon) < MIN_ABS);
}

constexpr float toRad(float degrees) {
  return degrees * PI / 180.0f;
}

inline float planarDistanceKm(float lat1, float lon1, float lat2, float lon2) {
  const float latRad = toRad((lat1 + lat2) / 2.0f);
  const float dLat   = toRad(lat2 - lat1);
  const float dLon   = toRad(lon2 - lon1);
  const float x      = dLon * cosf(latRad) * EARTH_RADIUS_M;
  const float y      = dLat * EARTH_RADIUS_M;
  return sqrtf(x * x + y * y) / 1000.0f;
}

/**
 * @brief Compare two TripStateData for UI-relevant changes
 * @return true if significant data changed
 */
inline bool isChanged(const TripStateData &s1, const TripStateData &s2) {
  // Use a small epsilon for speed/distance comparison to avoid fluttering
  constexpr float EPS = 0.05f;
  auto            eq  = [](float a, float b) { return fabsf(a - b) < EPS; };

  return !eq(s1.currentSpeed, s2.currentSpeed) || s1.status != s2.status ||
         !eq(s1.tripDistance, s2.tripDistance) || !eq(s1.maxSpeed, s2.maxSpeed) ||
         !eq(s1.avgSpeed, s2.avgSpeed) || s1.fixMode != s2.fixMode || !eq(s1.totalKm, s2.totalKm) ||
         (abs((long)(s1.totalElapsedMs - s2.totalElapsedMs)) >= 1000);
}

// ========================================
// In-place Computation
// ========================================

inline void computeTrip(TripStateDataEx &state, const GnssData &gnss, unsigned long now) {
  // Update timing
  if (state.lastUpdateTime == 0) {
    state.lastUpdateTime = now;
    state.updateStatus   = gnss.status;
    return;
  }
  const unsigned long dt = now - state.lastUpdateTime;
  state.lastUpdateTime   = now;

  // Update elapsed time
  if (state.status != TripStateData::Status::Paused) {
    state.totalElapsedMs += dt;
    if (state.status == TripStateData::Status::Moving) { state.totalMovingMs += dt; }
  }

  // Handle GNSS update
  if (gnss.status == UpdateStatus::Updated) {
    state.fixMode      = (SpFixMode)gnss.navData.posFixMode;
    const float rawKmh = calculateRawKmh(gnss.navData.velocity);
    const bool  fix    = hasFix(state.fixMode);
    const bool  moving = isMoving(fix, rawKmh);

    state.status       = determineStatus(state.status, moving);
    state.currentSpeed = calculateCurrentSpeed(state.status, rawKmh);

    // Coordinate update
    if (fix && isValidCoordinate(gnss.navData.latitude, gnss.navData.longitude)) {
      if (state.hasLastCoord) {
        float dist  = planarDistanceKm(state.lastLat, state.lastLon, gnss.navData.latitude,
                                       gnss.navData.longitude);
        float delta = calculateEffectiveDistance(dist);

        if (dist > MIN_DELTA) {
          state.lastLat = gnss.navData.latitude;
          state.lastLon = gnss.navData.longitude;
        }

        if (state.status != TripStateData::Status::Paused) { state.tripDistance += delta; }
        state.totalKm += delta;
      } else {
        state.lastLat      = gnss.navData.latitude;
        state.lastLon      = gnss.navData.longitude;
        state.hasLastCoord = true;
      }
    }

    if (state.currentSpeed > state.maxSpeed) { state.maxSpeed = state.currentSpeed; }
    state.updateStatus = UpdateStatus::Updated;
  } else {
    // Timeout check when GNSS is not updated
    if (isGnssTimedOut(now, gnss.timestamp)) {
      if (state.status == TripStateData::Status::Moving) {
        state.status       = TripStateData::Status::Stopped;
        state.currentSpeed = 0.0f;
        state.updateStatus = UpdateStatus::Updated;
      }
    }
  }

  // Recalculate derived patterns
  state.avgSpeed = calculateAverageSpeed(state.tripDistance, state.totalMovingMs);
}

} // namespace Pipeline
