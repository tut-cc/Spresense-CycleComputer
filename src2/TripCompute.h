#pragma once

#include "DataStructures.h"
#include <Arduino.h>
#include <GNSS.h>
#include <math.h>

// TripStateDataの拡張（内部状態を含む）
// Note: DataStructures.hのTripStateDataには含めず、ここで拡張版を定義
struct TripStateDataEx : public TripStateData {
  // 内部状態（座標履歴）
  float lastLat      = 0.0f;
  float lastLon      = 0.0f;
  bool  hasLastCoord = false;
};

namespace Pipeline {

// Trip.hからの定数
constexpr float         MS_PER_HOUR          = 60.0f * 60.0f * 1000.0f;
constexpr float         MIN_ABS              = 1e-6f;
constexpr float         MIN_DELTA            = 0.002f;
constexpr float         MAX_DELTA            = 1.0f;
constexpr float         EARTH_RADIUS_M       = 6378137.0f; // WGS84 [m]
constexpr float         MS_TO_KMH            = 3.6f;
constexpr float         MIN_MOVING_SPEED_KMH = 0.001f;
constexpr unsigned long SIGNAL_TIMEOUT_MS    = 2000;

// ========================================
// ヘルパー関数（純粋関数）
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

inline unsigned long calculateMovingMs(TripStateData::Status status, unsigned long totalMs,
                                       unsigned long dt) {
  return (status == TripStateData::Status::Moving) ? (totalMs + dt) : totalMs;
}

inline unsigned long calculateElapsedMs(TripStateData::Status status, unsigned long totalMs,
                                        unsigned long dt) {
  return (status != TripStateData::Status::Paused) ? (totalMs + dt) : totalMs;
}

inline float calculateEffectiveDistance(float dist) {
  if (dist > MIN_DELTA && dist <= MAX_DELTA) return dist;
  return 0.0f;
}

inline bool shouldUpdateLastCoordinate(float dist) {
  return dist > MIN_DELTA;
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

// ========================================
// 内部状態（Tripクラスのprivateメンバーに相当）
// ========================================

struct TripInternalState {
  float lastLat      = 0.0f;
  float lastLon      = 0.0f;
  bool  hasLastCoord = false;
};

// ========================================
// Odometer更新（純粋関数的に）
// ========================================

struct OdometerResult {
  float             deltaKm;
  TripInternalState newInternalState;
};

inline OdometerResult updateOdometer(const TripInternalState &internal, float lat, float lon,
                                     bool moving) {
  OdometerResult result;
  result.newInternalState = internal;
  result.deltaKm          = 0.0f;

  if (!internal.hasLastCoord) {
    result.newInternalState.lastLat      = lat;
    result.newInternalState.lastLon      = lon;
    result.newInternalState.hasLastCoord = true;
    return result;
  }

  // If not moving, no distance is accumulated for the odometer
  if (!moving) return result;

  const float dist  = planarDistanceKm(internal.lastLat, internal.lastLon, lat, lon);
  const float delta = calculateEffectiveDistance(dist);

  if (shouldUpdateLastCoordinate(dist)) {
    result.newInternalState.lastLat = lat;
    result.newInternalState.lastLon = lon;
  }

  result.deltaKm = delta;
  return result;
}

// ========================================
// Trip計算のメイン関数
// ========================================

// Note: この関数は内部状態を持つため、完全な純粋関数ではない
// 実際の実装では、TripInternalStateもTripStateDataに含めるか、
// 別途管理する必要がある

inline TripStateData computeTripWithInternal(const TripStateData     &oldState,
                                             const TripInternalState &oldInternal,
                                             const GnssData &gnss, unsigned long currentTime,
                                             TripInternalState &newInternal) {
  TripStateData newState = oldState;
  newInternal            = oldInternal;
  newState.updateStatus  = oldState.updateStatus;

  // 初回更新チェック
  if (oldState.lastUpdateTime == 0) {
    newState.lastUpdateTime = currentTime;
    if (newState.updateStatus < UpdateStatus::Updated) {
      newState.updateStatus = UpdateStatus::Updated;
    }
    return newState;
  }

  const unsigned long dt  = currentTime - oldState.lastUpdateTime;
  newState.lastUpdateTime = currentTime;

  // GNSS未更新かつ時間経過なしなら何もしない
  if (gnss.status == UpdateStatus::NoChange && dt == 0) { return newState; }

  // 経過時間の更新
  newState.totalMovingMs  = calculateMovingMs(oldState.status, oldState.totalMovingMs, dt);
  newState.totalElapsedMs = calculateElapsedMs(oldState.status, oldState.totalElapsedMs, dt);

  // GNSS未更新時
  if (gnss.status == UpdateStatus::NoChange) {
    // タイムアウトチェック
    if (isGnssTimedOut(currentTime, oldState.lastUpdateTime)) {
      if (newState.status != TripStateData::Status::Paused &&
          newState.status != TripStateData::Status::Stopped) {
        newState.status       = TripStateData::Status::Stopped;
        newState.currentSpeed = 0.0f;
        if (newState.updateStatus < UpdateStatus::Updated) {
          newState.updateStatus = UpdateStatus::Updated;
        }
      }
    }

    // 平均速度の更新
    float newAvgSpeed = calculateAverageSpeed(newState.tripDistance, newState.totalMovingMs);
    if (newAvgSpeed != oldState.avgSpeed) {
      newState.avgSpeed = newAvgSpeed;
      if (newState.updateStatus < UpdateStatus::Updated) {
        newState.updateStatus = UpdateStatus::Updated;
      }
    }

    return newState;
  }

  // GNSS更新時の処理
  newState.fixMode = (SpFixMode)gnss.navData.posFixMode;
  if (newState.fixMode != oldState.fixMode) {
    if (newState.updateStatus < UpdateStatus::Updated) {
      newState.updateStatus = UpdateStatus::Updated;
    }
  }
  const float rawKmh = calculateRawKmh(gnss.navData.velocity);
  const bool  fix    = hasFix(newState.fixMode);
  const bool  moving = isMoving(fix, rawKmh);

  newState.status       = determineStatus(oldState.status, moving);
  newState.currentSpeed = calculateCurrentSpeed(newState.status, rawKmh);

  // 座標が有効な場合、Odometer更新
  if (fix && isValidCoordinate(gnss.navData.latitude, gnss.navData.longitude)) {
    OdometerResult odoResult =
        updateOdometer(oldInternal, gnss.navData.latitude, gnss.navData.longitude, moving);
    newInternal = odoResult.newInternalState;

    if (newState.status != TripStateData::Status::Paused) {
      newState.tripDistance += odoResult.deltaKm;
    }
    newState.totalKm += odoResult.deltaKm;
  }

  // 最高速度の更新
  if (newState.currentSpeed > oldState.maxSpeed) { newState.maxSpeed = newState.currentSpeed; }

  // 平均速度の更新
  newState.avgSpeed = calculateAverageSpeed(newState.tripDistance, newState.totalMovingMs);

  // 何か変わったかチェック
  if (newState.currentSpeed != oldState.currentSpeed || newState.status != oldState.status ||
      newState.tripDistance != oldState.tripDistance || newState.maxSpeed != oldState.maxSpeed ||
      newState.avgSpeed != oldState.avgSpeed || newState.fixMode != oldState.fixMode) {
    if (newState.updateStatus < UpdateStatus::Updated) {
      newState.updateStatus = UpdateStatus::Updated;
    }
  }

  return newState;
}

// ========================================
// 公開API: TripStateDataExを使った簡潔な関数
// ========================================

inline TripStateDataEx computeTrip(const TripStateDataEx &oldState, const GnssData &gnss,
                                   unsigned long currentTime) {
  TripStateDataEx newState;

  // 内部状態を抽出
  TripInternalState oldInternal;
  oldInternal.lastLat      = oldState.lastLat;
  oldInternal.lastLon      = oldState.lastLon;
  oldInternal.hasLastCoord = oldState.hasLastCoord;

  // 計算
  TripInternalState newInternal;
  TripStateData     baseNewState =
      computeTripWithInternal(oldState, oldInternal, gnss, currentTime, newInternal);

  // 結果をコピー
  newState              = static_cast<TripStateDataEx &>(baseNewState);
  newState.lastLat      = newInternal.lastLat;
  newState.lastLon      = newInternal.lastLon;
  newState.hasLastCoord = newInternal.hasLastCoord;

  return newState;
}

} // namespace Pipeline
