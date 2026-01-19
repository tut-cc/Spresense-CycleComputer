#pragma once

#include "../common/DataStructures.h"
#include <Arduino.h>
#include <GNSS.h>
#include <math.h>

namespace Pipeline {

constexpr float         MS_PER_HOUR          = 3600000.0f;
constexpr float         MIN_ABS              = 1e-6f;
constexpr float         MIN_DELTA            = 0.002f;
constexpr float         MAX_DELTA            = 1.0f;
constexpr float         EARTH_RADIUS_M       = 6378137.0f;
constexpr float         MS_TO_KMH            = 3.6f;
constexpr float         MIN_MOVING_SPEED_KMH = 0.5f; // Adjusted from 0.001f for stability
constexpr unsigned long SIGNAL_TIMEOUT_MS    = 3000;

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

inline bool isValidCoordinate(float lat, float lon) {
  return !(fabsf(lat) < MIN_ABS && fabsf(lon) < MIN_ABS);
}

inline bool isChanged(const TripStateData &s1, const TripStateData &s2) {
  constexpr float EPS = 0.05f;
  auto            eq  = [](float a, float b) { return fabsf(a - b) < EPS; };

  return !eq(s1.currentSpeed, s2.currentSpeed) || s1.status != s2.status ||
         !eq(s1.tripDistance, s2.tripDistance) || !eq(s1.maxSpeed, s2.maxSpeed) ||
         !eq(s1.avgSpeed, s2.avgSpeed) || s1.fixMode != s2.fixMode || !eq(s1.totalKm, s2.totalKm) ||
         (abs((long)(s1.totalElapsedMs - s2.totalElapsedMs)) >= 1000);
}

inline void updateTimeAndDistance(TripStateDataEx &state, unsigned long dt) {
  if (state.status == TripStateData::Status::Paused) return;

  state.totalElapsedMs += dt;

  if (state.status == TripStateData::Status::Moving) {
    state.totalMovingMs += dt;

    const float dDist = state.currentSpeed * (static_cast<float>(dt) / MS_PER_HOUR);

    state.distanceResidue += dDist;
    if (state.distanceResidue >= 0.001f) {
      state.tripDistance += state.distanceResidue;
      state.totalKm += state.distanceResidue;
      state.distanceResidue = 0.0f;
    }
  }
}

inline void handleGnssUpdate(TripStateDataEx &state, const GnssData &gnss) {
  state.fixMode      = (SpFixMode)gnss.navData.posFixMode;
  const float rawKmh = calculateRawKmh(gnss.navData.velocity);
  const bool  fix    = hasFix(state.fixMode);
  const bool  moving = isMoving(fix, rawKmh);

  state.status       = determineStatus(state.status, moving);
  state.currentSpeed = calculateCurrentSpeed(state.status, rawKmh);

  if (fix && isValidCoordinate(gnss.navData.latitude, gnss.navData.longitude)) {
    state.lastLat      = gnss.navData.latitude;
    state.lastLon      = gnss.navData.longitude;
    state.hasLastCoord = true;
  }

  if (state.currentSpeed > state.maxSpeed) { state.maxSpeed = state.currentSpeed; }
  state.updateStatus = UpdateStatus::Updated;
}

inline void handleGnssTimeout(TripStateDataEx &state, unsigned long now,
                              unsigned long gnssTimestamp) {
  if (isGnssTimedOut(now, gnssTimestamp)) {
    if (state.status == TripStateData::Status::Moving) {
      state.status       = TripStateData::Status::Stopped;
      state.currentSpeed = 0.0f;
      state.updateStatus = UpdateStatus::Updated;
    }
  }
}

inline void computeTrip(TripStateDataEx &state, const GnssData &gnss, unsigned long now) {
  if (state.lastUpdateTime == 0) {
    state.lastUpdateTime = now;
    state.updateStatus   = gnss.status;
    return;
  }

  const unsigned long dt = now - state.lastUpdateTime;
  state.lastUpdateTime   = now;

  updateTimeAndDistance(state, dt);

  if (gnss.status == UpdateStatus::Updated) handleGnssUpdate(state, gnss);
  else handleGnssTimeout(state, now, gnss.timestamp);

  state.avgSpeed = calculateAverageSpeed(state.tripDistance, state.totalMovingMs);
}

} // namespace Pipeline
