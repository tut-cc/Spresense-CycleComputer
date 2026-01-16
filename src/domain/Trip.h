#pragma once

#include <Arduino.h>
#include <GNSS.h>
#include <math.h>

// ==========================================
// Trip
// ==========================================
class Trip {
private:
  // Speedometer members
  float currentSpeed = 0.0f;
  float maxSpeed     = 0.0f;
  float avgSpeed     = 0.0f;

  // Odometer members
  float totalKm      = 0.0f;
  float lastLat      = 0.0f;
  float lastLon      = 0.0f;
  bool  hasLastCoord = false;

  // Stopwatch members
  unsigned long totalMovingMs  = 0;
  unsigned long totalElapsedMs = 0;
  bool          isPaused       = false;

  // Trip specific members
  float         tripDistance  = 0.0f;
  unsigned long lastUpdateMs  = 0;
  bool          hasLastUpdate = false;

  // Constants
  static constexpr float MS_PER_HOUR          = 60.0f * 60.0f * 1000.0f;
  static constexpr float MIN_ABS              = 1e-6f;
  static constexpr float MIN_DELTA            = 0.002f;
  static constexpr float MAX_DELTA            = 1.0f;
  static constexpr float EARTH_RADIUS_M       = 6378137.0f; // WGS84 [m]
  static constexpr float MS_TO_KMH            = 3.6f;
  static constexpr float MIN_MOVING_SPEED_KMH = 0.001f;

public:
  void begin() {
    reset();
  }

  void update(const SpNavData &navData, unsigned long currentMillis) {
    if (!hasLastUpdate) {
      lastUpdateMs  = currentMillis;
      hasLastUpdate = true;
      return;
    }

    const unsigned long dt = currentMillis - lastUpdateMs;
    lastUpdateMs           = currentMillis;

    const float rawKmh   = navData.velocity * MS_TO_KMH;
    const bool  hasFix   = (navData.posFixMode == Fix2D || navData.posFixMode == Fix3D);
    const bool  isMoving = hasFix && (MIN_MOVING_SPEED_KMH < rawKmh); // Anti-GPS noise
    const float speedKmh = isMoving ? rawKmh : 0.0f;

    // Update Stopwatch logic
    if (isMoving) { totalMovingMs += dt; }
    if (!isPaused) totalElapsedMs += dt;

    // Update Odometer logic
    float deltaKm = 0.0f;
    if (hasFix) { deltaKm = updateOdometer(navData.latitude, navData.longitude, isMoving); }
    tripDistance += deltaKm;

    // Update Speedometer logic
    currentSpeed = speedKmh;
    if (maxSpeed < currentSpeed) maxSpeed = currentSpeed;
    if (0 < totalMovingMs) avgSpeed = tripDistance / (totalMovingMs / MS_PER_HOUR);
  }

  void resetTrip() {
    totalElapsedMs = 0;
    tripDistance   = 0.0f;
    lastUpdateMs   = 0;
    hasLastUpdate  = false;

    // Also reset speed stats and moving time for the trip
    currentSpeed  = 0.0f;
    maxSpeed      = 0.0f;
    avgSpeed      = 0.0f;
    totalMovingMs = 0;
  }

  void resetOdometer() {
    // Reset Odometer
    totalKm      = 0.0f;
    lastLat      = 0.0f;
    lastLon      = 0.0f;
    hasLastCoord = false;
  }

  void reset() {
    resetTrip();
    resetOdometer();
  }

  void pause() {
    isPaused = !isPaused;
  }

  void restore(float totalDist, float tripDist, unsigned long movingTime, float maxSpd) {
    totalKm       = totalDist;
    tripDistance  = tripDist;
    totalMovingMs = movingTime;
    maxSpeed      = maxSpd;
  }

  // Getters
  float getTripDistance() const {
    return tripDistance;
  }
  float getTotalDistance() const {
    return totalKm;
  }
  unsigned long getTotalMovingTimeMs() const {
    return totalMovingMs;
  }
  unsigned long getElapsedTimeMs() const {
    return totalElapsedMs;
  }
  float getSpeed() const {
    return currentSpeed;
  }
  float getAvgSpeed() const {
    return avgSpeed;
  }
  float getMaxSpeed() const {
    return maxSpeed;
  }

private:
  // Odometer helper methods
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
        // Too far jump, just update position to reset baseline
        lastLat = lat;
        lastLon = lon;
      } else if (dist > MIN_DELTA) {
        // Valid movement
        deltaKm = dist;
        totalKm += deltaKm;
        lastLat = lat;
        lastLon = lon;
      }
      // If dist <= MIN_DELTA, keep old lastLat/lastLon to accumulate distance
    }

    return deltaKm;
  }
};
