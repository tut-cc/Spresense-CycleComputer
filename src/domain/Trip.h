#pragma once

#include "NavData.h"
#include <Arduino.h>
#include <math.h>

// ==========================================
// Speedometer
// ==========================================
class Speedometer {
private:
  float curKmh = 0.0f;
  float maxKmh = 0.0f;
  float avgKmh = 0.0f;

  static constexpr float MS_PER_HOUR = 60.0f * 60.0f * 1000.0f;

public:
  void update(float curKmh, unsigned long movingTimeMs, float totalKm) {
    this->curKmh = curKmh;
    if (maxKmh < curKmh) maxKmh = curKmh;
    if (0 < movingTimeMs) avgKmh = totalKm / (movingTimeMs / MS_PER_HOUR);
  }

  void reset() {
    curKmh = 0.0f;
    maxKmh = 0.0f;
    avgKmh = 0.0f;
  }

  float getCur() const {
    return curKmh;
  }

  float getMax() const {
    return maxKmh;
  }

  float getAvg() const {
    return avgKmh;
  }
};

// ==========================================
// Odometer
// ==========================================
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

  static constexpr float EARTH_RADIUS_M = 6378137.0f; // WGS84 [m]
  static float           planarDistanceKm(float lat1, float lon1, float lat2, float lon2) {
    const float latRad = toRad((lat1 + lat2) / 2.0f);
    const float dLat   = toRad(lat2 - lat1);
    const float dLon   = toRad(lon2 - lon1);
    const float x      = dLon * cosf(latRad) * EARTH_RADIUS_M;
    const float y      = dLat * EARTH_RADIUS_M;
    return sqrtf(x * x + y * y) / 1000.0f; // km
  }
};

// ==========================================
// Stopwatch
// ==========================================
class Stopwatch {
private:
  unsigned long movingTimeMs = 0;
  unsigned long totalTimeMs  = 0;
  bool          isPaused     = false;

public:
  void update(bool isMoving, unsigned long dt) {
    if (isMoving) movingTimeMs += dt;
    if (!isPaused) totalTimeMs += dt;
  }

  void resetTotalTime() {
    totalTimeMs = 0;
  }

  void resetMovingTime() {
    movingTimeMs = 0;
  }

  void setMovingTime(unsigned long ms) {
    movingTimeMs = ms;
  }

  void reset() {
    resetTotalTime();
    resetMovingTime();
  }

  void togglePause() {
    isPaused = !isPaused;
  }

  unsigned long getMovingTimeMs() const {
    return movingTimeMs;
  }

  unsigned long getElapsedTimeMs() const {
    return totalTimeMs;
  }
};

// ==========================================
// Trip
// ==========================================
class Trip {
public:
private:
  Speedometer speedometer;
  Odometer    odometer;
  Stopwatch   stopwatch;

public:
  float getTripDistance() const {
    return tripDistance;
  }

  float getTotalDistance() const {
    return odometer.getTotalDistance();
  }

  unsigned long getMovingTimeMs() const {
    return stopwatch.getMovingTimeMs();
  }

  unsigned long getElapsedTimeMs() const {
    return stopwatch.getElapsedTimeMs();
  }

  float getSpeed() const {
    return speedometer.getCur();
  }

  float getAvgSpeed() const {
    return speedometer.getAvg();
  }

  float getMaxSpeed() const {
    return speedometer.getMax();
  }

  void restore(float totalDist, float tripDist, unsigned long movingTime) {
    odometer.setTotalDistance(totalDist);
    tripDistance = tripDist;
    stopwatch.setMovingTime(movingTime);
  }

private:
  float         tripDistance = 0.0f;
  unsigned long lastMillis;

  bool hasLastMillis;

  static constexpr float MS_TO_KMH            = 3.6f;
  static constexpr float MIN_MOVING_SPEED_KMH = 0.001f;

public:
  void begin() {
    reset();
  }

  void update(const NavData &navData, unsigned long currentMillis) {
    if (!hasLastMillis) {
      lastMillis    = currentMillis;
      hasLastMillis = true;
      return;
    }

    const unsigned long dt = currentMillis - lastMillis;
    lastMillis             = currentMillis;

    const float rawKmh   = navData.velocity * MS_TO_KMH;
    const bool  hasFix   = navData.fixType != FixType::NoFix;
    const bool  isMoving = hasFix && (MIN_MOVING_SPEED_KMH < rawKmh); // Anti-GPS noise
    const float speedKmh = isMoving ? rawKmh : 0.0f;

    stopwatch.update(isMoving, dt);

    float deltaKm = 0.0f;
    if (hasFix) { deltaKm = odometer.update(navData.latitude, navData.longitude, isMoving); }
    tripDistance += deltaKm;

    speedometer.update(speedKmh, stopwatch.getMovingTimeMs(), tripDistance);
  }

  void resetTrip() {
    stopwatch.resetTotalTime();
    tripDistance  = 0.0f;
    lastMillis    = 0;
    hasLastMillis = false;
  }

  void resetOdometerAndMovingTime() {
    odometer.reset();
    tripDistance = 0.0f;
    stopwatch.resetMovingTime();
  }

  void reset() {
    resetTrip();
    resetOdometerAndMovingTime();
  }

  void pause() {
    stopwatch.togglePause();
  }
};
