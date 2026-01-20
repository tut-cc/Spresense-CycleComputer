#pragma once

/**
 * @file TripLogic.h
 * @brief トリップデータの計算ロジック
 *
 * GNSS情報から速度、距離、時間を計算し、トリップ状態を更新します。
 * 移動判定、タイムアウト処理、平均速度計算などの純粋関数を提供。
 */

#include "../common/Config.h"
#include "../common/DataStructures.h"
#include <Arduino.h>
#include <GNSS.h>
#include <math.h>

namespace TripLogic {

/// ミリ秒から時間への変換係数
constexpr float MS_PER_HOUR = 3600000.0f;

/// 浮動小数点比較用の最小値
constexpr float MIN_ABS = 1e-6f;

/// m/s から km/h への変換係数
constexpr float MS_TO_KMH = 3.6f;

/// 移動判定の最低速度 (Config.hから参照)
constexpr float MIN_MOVING_SPEED_KMH = Config::Gnss::MIN_MOVING_SPEED_KMH;

/// GNSS信号ロストのタイムアウト (Config.hから参照)
constexpr unsigned long SIGNAL_TIMEOUT_MS = Config::Gnss::SIGNAL_TIMEOUT_MS;

inline float calculateRawKmh(float velocity) { return velocity * MS_TO_KMH; }
inline bool  hasFix(SpFixMode mode) { return (mode == Fix2D || mode == Fix3D); }
inline bool  isMoving(bool fix, float rawKmh) { return fix && (rawKmh > MIN_MOVING_SPEED_KMH); }

inline TripStateBase::Status determineStatus(TripStateBase::Status currentStatus, bool moving) {
  if (currentStatus == TripStateBase::Status::Paused) return TripStateBase::Status::Paused;
  return moving ? TripStateBase::Status::Moving : TripStateBase::Status::Stopped;
}

inline float calculateCurrentSpeed(TripStateBase::Status status, float rawKmh) {
  return (status == TripStateBase::Status::Moving) ? rawKmh : 0.0f;
}

inline bool isGnssTimedOut(unsigned long currentMs, unsigned long lastUpdateMs) {
  return (currentMs - lastUpdateMs > SIGNAL_TIMEOUT_MS);
}

inline float calculateAverageSpeed(float tripDistance, unsigned long totalMovingMs) {
  if (totalMovingMs == 0) return 0.0f;
  return tripDistance / (totalMovingMs / MS_PER_HOUR);
}

inline bool isChanged(const TripStateBase &s1, const TripStateBase &s2) {
  constexpr float SPEED_EPS    = 0.05f;
  constexpr float DISTANCE_EPS = 0.001f;
  constexpr long  TIME_EPS_MS  = 1000;

  auto floatEq = [](float a, float b, float eps) { return fabsf(a - b) < eps; };

  const bool speedChanged = !floatEq(s1.speed.current, s2.speed.current, SPEED_EPS) ||
                            !floatEq(s1.speed.max, s2.speed.max, SPEED_EPS) ||
                            !floatEq(s1.speed.avg, s2.speed.avg, SPEED_EPS);

  const bool distanceChanged = !floatEq(s1.distance.trip, s2.distance.trip, DISTANCE_EPS) ||
                               !floatEq(s1.distance.total, s2.distance.total, DISTANCE_EPS);

  const bool timeChanged = (abs((long)(s1.time.elapsed - s2.time.elapsed)) >= TIME_EPS_MS);

  const bool statusChanged  = (s1.status != s2.status);
  const bool fixModeChanged = (s1.fixMode != s2.fixMode);

  return speedChanged || distanceChanged || timeChanged || statusChanged || fixModeChanged;
}

inline void updateTimeAndDistance(TripState &state, unsigned long dt) {
  if (state.status == TripStateBase::Status::Paused) return;

  state.time.elapsed += dt;

  if (state.status == TripStateBase::Status::Moving) {
    state.time.moving += dt;
    const float dDist = state.speed.current * (static_cast<float>(dt) / MS_PER_HOUR);
    state.distanceResidue += dDist;
    if (state.distanceResidue >= 0.001f) {
      state.distance.trip += state.distanceResidue;
      state.distance.total += state.distanceResidue;
      state.distanceResidue = 0.0f;
    }
  }
}

inline void handleGnssUpdate(TripState &state, const GnssData &gnss) {
  state.fixMode      = (SpFixMode)gnss.navData.posFixMode;
  const float rawKmh = calculateRawKmh(gnss.navData.velocity);
  const bool  fix    = hasFix(state.fixMode);
  const bool  moving = isMoving(fix, rawKmh);

  state.status        = determineStatus(state.status, moving);
  state.speed.current = calculateCurrentSpeed(state.status, rawKmh);

  if (state.speed.current > state.speed.max) state.speed.max = state.speed.current;
  state.updateStatus = UpdateStatus::Updated;
}

inline void handleGnssTimeout(TripState &state, unsigned long now, unsigned long gnssTimestamp) {
  if (isGnssTimedOut(now, gnssTimestamp)) {
    if (state.status == TripStateBase::Status::Moving) {
      state.status        = TripStateBase::Status::Stopped;
      state.speed.current = 0.0f;
      state.updateStatus  = UpdateStatus::Updated;
    }
  }
}

inline void computeTrip(TripState &state, const GnssData &gnss, unsigned long now) {
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

  state.speed.avg = calculateAverageSpeed(state.distance.trip, state.time.moving);
}

} // namespace TripLogic
