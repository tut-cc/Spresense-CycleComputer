#pragma once
/**
 * @file TripLogic.h
 * @brief 走行データの計算ロジック
 *
 * GNSSデータから速度・距離・時間を計算し、TripStateを更新します。
 * 純粋関数として設計されており、テストが容易です。
 */

#include "../common/DataStructures.h"
#include <Arduino.h>
#include <GNSS.h>
#include <math.h>

namespace TripLogic {

// ==================== 定数定義 ====================
constexpr float         MS_PER_HOUR          = 3600000.0f; ///< 1時間のミリ秒数
constexpr float         MIN_ABS              = 1e-6f;      ///< 最小絶対値
constexpr float         MS_TO_KMH            = 3.6f;       ///< m/s → km/h 変換係数
constexpr float         MIN_MOVING_SPEED_KMH = 0.5f;       ///< 走行判定の最低速度
constexpr unsigned long SIGNAL_TIMEOUT_MS    = 3000;       ///< GPS信号タイムアウト

// ==================== ユーティリティ関数 ====================

/** @brief 速度をm/sからkm/hに変換 */
inline float calculateRawKmh(float velocity) { return velocity * MS_TO_KMH; }

/** @brief GPS信号が有効か判定 (2Dまたは3D fix) */
inline bool hasFix(SpFixMode mode) { return (mode == Fix2D || mode == Fix3D); }

/** @brief 走行中か判定 (GPS有効かつ最低速度以上) */
inline bool isMoving(bool fix, float rawKmh) { return fix && (rawKmh > MIN_MOVING_SPEED_KMH); }

/**
 * @brief 走行状態を決定
 * @note ポーズ中は状態を維持
 */
inline TripStateBase::Status determineStatus(TripStateBase::Status currentStatus, bool moving) {
  if (currentStatus == TripStateBase::Status::Paused) return TripStateBase::Status::Paused;
  return moving ? TripStateBase::Status::Moving : TripStateBase::Status::Stopped;
}

/** @brief 表示用の現在速度を計算 (走行中のみ速度を返す) */
inline float calculateCurrentSpeed(TripStateBase::Status status, float rawKmh) {
  return (status == TripStateBase::Status::Moving) ? rawKmh : 0.0f;
}

/** @brief GPS信号がタイムアウトしたか判定 */
inline bool isGnssTimedOut(unsigned long currentMs, unsigned long lastUpdateMs) {
  return (currentMs - lastUpdateMs > SIGNAL_TIMEOUT_MS);
}

/** @brief 平均速度を計算 */
inline float calculateAverageSpeed(float tripDistance, unsigned long totalMovingMs) {
  if (totalMovingMs == 0) return 0.0f;
  return tripDistance / (totalMovingMs / MS_PER_HOUR);
}

/**
 * @brief 2つの走行状態に変化があるか判定
 * @return 変化があればtrue
 *
 * UI更新判定に使用。微小な変化は無視します。
 */
inline bool isChanged(const TripStateBase &s1, const TripStateBase &s2) {
  constexpr float EPS = 0.05f; // 許容誤差
  auto            eq  = [](float a, float b) { return fabsf(a - b) < EPS; };

  return !eq(s1.speed.current, s2.speed.current) || s1.status != s2.status ||
         !eq(s1.distance.trip, s2.distance.trip) || !eq(s1.speed.max, s2.speed.max) ||
         !eq(s1.speed.avg, s2.speed.avg) || s1.fixMode != s2.fixMode ||
         !eq(s1.distance.total, s2.distance.total) ||
         (abs((long)(s1.time.elapsed - s2.time.elapsed)) >= 1000);
}

/**
 * @brief 時間と距離を更新
 *
 * 距離計算では残差を蓄積して精度を確保します。
 * 0.001km未満の距離は累積して、達した時点で加算します。
 */
inline void updateTimeAndDistance(TripState &state, unsigned long dt) {
  if (state.status == TripStateBase::Status::Paused) return;

  state.time.elapsed += dt;

  if (state.status == TripStateBase::Status::Moving) {
    state.time.moving += dt;

    // 速度×時間で距離を計算
    const float dDist = state.speed.current * (static_cast<float>(dt) / MS_PER_HOUR);

    // 残差に加算し、0.001km以上になったら本体に反映
    state.distanceResidue += dDist;
    if (state.distanceResidue >= 0.001f) {
      state.distance.trip += state.distanceResidue;
      state.distance.total += state.distanceResidue;
      state.distanceResidue = 0.0f;
    }
  }
}

/**
 * @brief GNSSデータ更新時の処理
 */
inline void handleGnssUpdate(TripState &state, const GnssData &gnss) {
  state.fixMode      = (SpFixMode)gnss.navData.posFixMode;
  const float rawKmh = calculateRawKmh(gnss.navData.velocity);
  const bool  fix    = hasFix(state.fixMode);
  const bool  moving = isMoving(fix, rawKmh);

  state.status        = determineStatus(state.status, moving);
  state.speed.current = calculateCurrentSpeed(state.status, rawKmh);

  // 最高速度を更新
  if (state.speed.current > state.speed.max) { state.speed.max = state.speed.current; }
  state.updateStatus = UpdateStatus::Updated;
}

/**
 * @brief GPS信号タイムアウト時の処理
 */
inline void handleGnssTimeout(TripState &state, unsigned long now, unsigned long gnssTimestamp) {
  if (isGnssTimedOut(now, gnssTimestamp)) {
    if (state.status == TripStateBase::Status::Moving) {
      state.status        = TripStateBase::Status::Stopped;
      state.speed.current = 0.0f;
      state.updateStatus  = UpdateStatus::Updated;
    }
  }
}

/**
 * @brief 走行データを計算（メイン処理）
 * @param state 走行状態（更新される）
 * @param gnss GNSSデータ
 * @param now 現在時刻
 */
inline void computeTrip(TripState &state, const GnssData &gnss, unsigned long now) {
  // 初回呼び出し時の初期化
  if (state.lastUpdateTime == 0) {
    state.lastUpdateTime = now;
    state.updateStatus   = gnss.status;
    return;
  }

  // 前回からの経過時間
  const unsigned long dt = now - state.lastUpdateTime;
  state.lastUpdateTime   = now;

  // 時間と距離を更新
  updateTimeAndDistance(state, dt);

  // GNSSデータに基づく更新、またはタイムアウト処理
  if (gnss.status == UpdateStatus::Updated) handleGnssUpdate(state, gnss);
  else handleGnssTimeout(state, now, gnss.timestamp);

  // 平均速度を再計算
  state.speed.avg = calculateAverageSpeed(state.distance.trip, state.time.moving);
}

} // namespace TripLogic
