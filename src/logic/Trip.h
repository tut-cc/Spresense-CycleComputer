#pragma once

#include <Arduino.h>
#include <GNSS.h>
#include <math.h>

constexpr float         MS_PER_HOUR          = 60.0f * 60.0f * 1000.0f;
constexpr float         MIN_ABS              = 1e-6f;
constexpr float         MIN_DELTA            = 0.002f;
constexpr float         MAX_DELTA            = 1.0f;
constexpr float         EARTH_RADIUS_M       = 6378137.0f; // WGS84 [m]
constexpr float         MS_TO_KMH            = 3.6f;
constexpr float         MIN_MOVING_SPEED_KMH = 0.001f;
constexpr unsigned long SIGNAL_TIMEOUT_MS    = 2000;

class Trip {
public:
  enum class Status { Stopped, Moving, Paused };

  struct State {
    float         currentSpeed   = 0.0f;
    float         maxSpeed       = 0.0f;
    float         avgSpeed       = 0.0f;
    float         totalKm        = 0.0f;
    float         tripDistance   = 0.0f;
    unsigned long totalMovingMs  = 0;
    unsigned long totalElapsedMs = 0;
    Status        status         = Status::Stopped;
  };

private:
  State state;

  float lastLat      = 0.0f;
  float lastLon      = 0.0f;
  bool  hasLastCoord = false;

  unsigned long lastUpdateMs     = 0;
  unsigned long lastGnssUpdateMs = 0;
  bool          hasLastUpdate    = false;

public:
  void begin() {
    reset();
  }

  void update(const SpNavData &navData, unsigned long currentMillis, bool isGnssUpdated) {
    if (!hasLastUpdate) {
      initializeUpdateTime(currentMillis);
      return;
    }

    const unsigned long dt = currentMillis - lastUpdateMs;
    lastUpdateMs           = currentMillis;

    updateElapsedTimes(dt);

    if (isGnssUpdated) {
      processGnssUpdate(navData, currentMillis);
      // GNSS更新時は常に平均速度を再計算
      state.avgSpeed = calculateAverageSpeed(state.tripDistance, state.totalMovingMs);
    } else {
      handleGnssTimeout(currentMillis);
      // GNSS未更新時でも、1秒（1000ms）ごとに平均速度を更新（移動時間による減衰を反映）
      if (dt >= 1000 || (currentMillis % 1000 < dt)) {
        state.avgSpeed = calculateAverageSpeed(state.tripDistance, state.totalMovingMs);
      }
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
    state.status        = Status::Stopped;
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
    state.status = (state.status == Status::Paused) ? Status::Stopped : Status::Paused;
  }

  void restore(float totalDist, float tripDist, unsigned long movingTime, float maxSpd) {
    state.totalKm       = totalDist;
    state.tripDistance  = tripDist;
    state.totalMovingMs = movingTime;
    state.maxSpeed      = maxSpd;
    state.status        = Status::Stopped;
  }

  const State &getState() const {
    return state;
  }

private:
  void initializeUpdateTime(unsigned long currentMillis) {
    lastUpdateMs     = currentMillis;
    lastGnssUpdateMs = currentMillis;
    hasLastUpdate    = true;
  }

  void updateElapsedTimes(unsigned long dt) {
    state.totalMovingMs  = calculateMovingMs(state.status, state.totalMovingMs, dt);
    state.totalElapsedMs = calculateElapsedMs(state.status, state.totalElapsedMs, dt);
  }

  void processGnssUpdate(const SpNavData &navData, unsigned long currentMillis) {
    lastGnssUpdateMs = currentMillis;

    const float rawKmh = calculateRawKmh(navData.velocity);
    const bool  fix    = hasFix((SpFixMode)navData.posFixMode);
    const bool  moving = isMoving(fix, rawKmh);

    state.status       = determineStatus(state.status, moving);
    state.currentSpeed = calculateCurrentSpeed(state.status, rawKmh);

    if (fix && isValidCoordinate(navData.latitude, navData.longitude)) {
      float deltaKm = updateOdometer(navData.latitude, navData.longitude, moving);
      if (state.status != Status::Paused) { state.tripDistance += deltaKm; }
    }

    state.maxSpeed = fmaxf(state.maxSpeed, state.currentSpeed);
  }

  void handleGnssTimeout(unsigned long currentMillis) {
    if (isGnssTimedOut(currentMillis, lastGnssUpdateMs)) {
      if (state.status != Status::Paused) { state.status = Status::Stopped; }
      state.currentSpeed = 0.0f;
    }
  }

  float updateOdometer(float lat, float lon, bool moving) {
    if (!hasLastCoord) {
      updateLastCoordinate(lat, lon);
      hasLastCoord = true;
      return 0.0f;
    }

    // If not moving, no distance is accumulated for the odometer
    if (!moving) return 0.0f;

    const float dist  = planarDistanceKm(lastLat, lastLon, lat, lon);
    const float delta = calculateEffectiveDistance(dist);

    if (shouldUpdateLastCoordinate(dist)) { updateLastCoordinate(lat, lon); }

    state.totalKm += delta;
    return delta;
  }

  void updateLastCoordinate(float lat, float lon) {
    lastLat = lat;
    lastLon = lon;
  }

  static float calculateRawKmh(float velocity) {
    return velocity * MS_TO_KMH;
  }

  static bool hasFix(SpFixMode mode) {
    return (mode == Fix2D || mode == Fix3D);
  }

  static bool isMoving(bool fix, float rawKmh) {
    return fix && (rawKmh > MIN_MOVING_SPEED_KMH);
  }

  static Status determineStatus(Status currentStatus, bool moving) {
    if (currentStatus == Status::Paused) return Status::Paused;
    return moving ? Status::Moving : Status::Stopped;
  }

  static float calculateCurrentSpeed(Status status, float rawKmh) {
    return (status == Status::Moving) ? rawKmh : 0.0f;
  }

  static bool isGnssTimedOut(unsigned long currentMs, unsigned long lastUpdateMs) {
    return (currentMs - lastUpdateMs > SIGNAL_TIMEOUT_MS);
  }

  static float calculateAverageSpeed(float tripDistance, unsigned long totalMovingMs) {
    if (totalMovingMs == 0) return 0.0f;
    return tripDistance / (totalMovingMs / MS_PER_HOUR);
  }

  static unsigned long calculateMovingMs(Status status, unsigned long totalMs, unsigned long dt) {
    return (status == Status::Moving) ? (totalMs + dt) : totalMs;
  }

  static unsigned long calculateElapsedMs(Status status, unsigned long totalMs, unsigned long dt) {
    return (status != Status::Paused) ? (totalMs + dt) : totalMs;
  }

  static float calculateEffectiveDistance(float dist) {
    if (dist > MIN_DELTA && dist <= MAX_DELTA) return dist;
    return 0.0f;
  }

  static bool shouldUpdateLastCoordinate(float dist) {
    return dist > MIN_DELTA;
  }

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
    return sqrtf(x * x + y * y) / 1000.0f;
  }
};
