#pragma once

#include <Arduino.h>
#include <GNSS.h>
#include <math.h>

class Trip {
public:
  struct State {
    float         currentSpeed   = 0.0f;
    float         maxSpeed       = 0.0f;
    float         avgSpeed       = 0.0f;
    float         totalKm        = 0.0f;
    float         tripDistance   = 0.0f;
    unsigned long totalMovingMs  = 0;
    unsigned long totalElapsedMs = 0;
    bool          isPaused       = false;
  };

private:
  State state;

  float lastLat      = 0.0f;
  float lastLon      = 0.0f;
  bool  hasLastCoord = false;

  unsigned long lastUpdateMs     = 0;
  unsigned long lastGnssUpdateMs = 0;
  bool          hasLastUpdate    = false;

  static constexpr float         MS_PER_HOUR          = 60.0f * 60.0f * 1000.0f;
  static constexpr float         MIN_ABS              = 1e-6f;
  static constexpr float         MIN_DELTA            = 0.002f;
  static constexpr float         MAX_DELTA            = 1.0f;
  static constexpr float         EARTH_RADIUS_M       = 6378137.0f; // WGS84 [m]
  static constexpr float         MS_TO_KMH            = 3.6f;
  static constexpr float         MIN_MOVING_SPEED_KMH = 0.001f;
  static constexpr unsigned long SIGNAL_TIMEOUT_MS    = 2000;

public:
  void begin() {
    reset();
  }

  void update(const SpNavData &navData, unsigned long currentMillis, bool isGnssUpdated) {
    if (!hasLastUpdate) {
      lastUpdateMs     = currentMillis;
      lastGnssUpdateMs = currentMillis;
      hasLastUpdate    = true;
      return;
    }

    const unsigned long dt = currentMillis - lastUpdateMs;
    lastUpdateMs           = currentMillis;

    // 1. Update time-based state (always runs)
    if (state.currentSpeed > 0 && !state.isPaused) { state.totalMovingMs += dt; }
    if (!state.isPaused) { state.totalElapsedMs += dt; }

    // 2. Process GNSS data only if updated
    if (isGnssUpdated) {
      lastGnssUpdateMs = currentMillis;

      const float rawKmh   = navData.velocity * MS_TO_KMH;
      const bool  hasFix   = (navData.posFixMode == Fix2D || navData.posFixMode == Fix3D);
      const bool  isMoving = hasFix && (MIN_MOVING_SPEED_KMH < rawKmh);

      state.currentSpeed = isMoving ? rawKmh : 0.0f;

      float deltaKm = 0.0f;
      if (hasFix) { deltaKm = updateOdometer(navData.latitude, navData.longitude, isMoving); }
      state.tripDistance += deltaKm;

      if (state.maxSpeed < state.currentSpeed) { state.maxSpeed = state.currentSpeed; }
    } else {
      // 3. Signal Timeout Check
      // If no valid GNSS data for a while, assume we stopped or lost signal
      if (currentMillis - lastGnssUpdateMs > SIGNAL_TIMEOUT_MS) { state.currentSpeed = 0.0f; }
    }

    // 4. Update Average Speed
    if (state.totalMovingMs > 0) {
      state.avgSpeed = state.tripDistance / (state.totalMovingMs / MS_PER_HOUR);
    }
  }

  void resetTrip() {
    state.totalElapsedMs = 0;
    state.tripDistance   = 0.0f;
    lastUpdateMs         = 0;
    lastGnssUpdateMs     = 0;
    hasLastUpdate        = false;

    state.currentSpeed  = 0.0f;
    state.maxSpeed      = 0.0f;
    state.avgSpeed      = 0.0f;
    state.totalMovingMs = 0;
  }

  void resetOdometer() {
    state.totalKm = 0.0f;
    lastLat       = 0.0f;
    lastLon       = 0.0f;
    hasLastCoord  = false;
  }

  void resetMaxSpeed() {
    state.maxSpeed = 0.0f;
  }

  void reset() {
    resetTrip();
    resetOdometer();
  }

  void pause() {
    state.isPaused = !state.isPaused;
  }

  void restore(float totalDist, float tripDist, unsigned long movingTime, float maxSpd) {
    state.totalKm       = totalDist;
    state.tripDistance  = tripDist;
    state.totalMovingMs = movingTime;
    state.maxSpeed      = maxSpd;
  }

  const State &getState() const {
    return state;
  }

private:
  static bool isValidCoordinate(float lat, float lon) {
    return !(fabsf(lat) < MIN_ABS && fabsf(lon) < MIN_ABS);
  }

  static constexpr float toRad(float degrees) {
    return degrees * PI / 180.0f;
  }

  static float planarDistanceKm(float lat1, float lon1, float lat2, float lon2) {
    const float latRad = toRad((lat1 + lat2) / 2.0f);
    const float dLat   = toRad(lat2 - lat1);
    const float dLon   = toRad(lon2 - lon1);
    const float x      = dLon * cosf(latRad) * EARTH_RADIUS_M;
    const float y      = dLat * EARTH_RADIUS_M;
    return sqrtf(x * x + y * y) / 1000.0f; // km
  }

  float updateOdometer(float lat, float lon, bool isMoving) {
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
      const float dist = planarDistanceKm(lastLat, lastLon, lat, lon);

      if (dist >= MAX_DELTA) {
        lastLat = lat;
        lastLon = lon;
      } else if (dist > MIN_DELTA) {
        deltaKm = dist;
        state.totalKm += deltaKm;
        lastLat = lat;
        lastLon = lon;
      }
    }

    return deltaKm;
  }
};
