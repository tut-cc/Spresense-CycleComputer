#pragma once

/**
 * @file TripState.h
 * @brief トリップ状態の保持と更新ロジック
 */

#include "../common/BaseTypes.h" // Mode, GnssData を使用
#include "../common/Config.h"
#include <Arduino.h>
#include <GNSS.h>

/// EEPROM保存データの有効性を検証するためのマジックナンバー
constexpr uint32_t SAVE_DATA_MAGIC_NUMBER = 0xDEADBEEF;
constexpr float    MS_TO_HOUR             = 3600000.0f;
constexpr float    DISTANCE_UNIT_KM       = 0.001f; // 1 meter

struct TripState;

/**
 * @brief EEPROMに保存される永続データ構造
 */
struct SaveData {
  uint32_t      magicNumber;
  float         totalDistance;
  float         tripDistance;
  unsigned long movingTimeMs;
  float         maxSpeed;
  float         voltage;
  uint32_t      crc;

  SaveData()
      : magicNumber(SAVE_DATA_MAGIC_NUMBER), totalDistance(0), tripDistance(0), movingTimeMs(0),
        maxSpeed(0), voltage(0), crc(0) {}

  SaveData(const TripState &state, float v);

  bool operator==(const SaveData &other) const {
    return (magicNumber == other.magicNumber) && (totalDistance == other.totalDistance) &&
           (tripDistance == other.tripDistance) && (movingTimeMs == other.movingTimeMs) &&
           (maxSpeed == other.maxSpeed) && (voltage == other.voltage);
  }

  bool operator!=(const SaveData &other) const { return !(*this == other); }
};

/**
 * @brief トリップの走行状態を管理する構造体
 */
struct TripState {
  enum class Status { Stopped, Moving, Paused };

  struct Speed {
    float current;
    float max;
    float avg;
  };

  struct Distance {
    float total;
    float trip;
  };

  struct Time {
    unsigned long elapsed;
    unsigned long moving;
  };

  Status        status          = Status::Stopped;
  SpFixMode     fixMode         = FixInvalid;
  Speed         speed           = {0, 0, 0};
  Distance      distance        = {0, 0};
  Time          time            = {0, 0};
  unsigned long lastUpdateTime  = 0;
  float         distanceResidue = 0.0f;

  TripState() = default;

  /// 保存データから復元
  TripState(const SaveData &saved) {
    clearAllData();
    distance.total = saved.totalDistance;
    distance.trip  = saved.tripDistance;
    time.moving    = saved.movingTimeMs;
    speed.max      = saved.maxSpeed;
  }

  /// 現在の状態とGNSSデータから次の状態を計算（コンストラクタによる更新）
  TripState(const TripState &prev, const GnssData &gnss, unsigned long now) : TripState(prev) {
    if (lastUpdateTime == 0) {
      lastUpdateTime = now;
      return;
    }

    const unsigned long dt = now - lastUpdateTime;
    updateAccumulations(dt);

    if (gnss.updated) {
      handleGnssUpdate(gnss);
    } else {
      handleGnssTimeout(now);
    }

    lastUpdateTime = now;
  }

  static float calculateRawKmh(float velocity) { return velocity * 3.6f; }
  static bool  hasFix(SpFixMode mode) { return (mode == Fix2D || mode == Fix3D); }
  static bool  isMoving(bool fix, float rawKmh) {
    return fix && (rawKmh > Config::Gnss::MIN_MOVING_SPEED_KMH);
  }

  bool isPaused() const { return status == Status::Paused; }
  bool isMoving() const { return status == Status::Moving; }

  void clearAllData() {
    speed           = {0, 0, 0};
    status          = Status::Stopped;
    time            = {0, 0};
    distance        = {0, 0};
    lastUpdateTime  = 0;
    distanceResidue = 0.0f;
  }

  void clearTripData() {
    speed.current   = 0.0f;
    status          = Status::Stopped;
    time.elapsed    = 0;
    distance.trip   = 0.0f;
    time.moving     = 0;
    speed.avg       = 0.0f;
    distanceResidue = 0.0f;
  }

  void resetMaxSpeed() { speed.max = 0.0f; }

  void updateAverageSpeed() {
    if (time.moving == 0) speed.avg = 0.0f;
    else speed.avg = distance.trip / (time.moving / MS_TO_HOUR);
  }

  bool operator!=(const TripState &other) const {
    constexpr float SPEED_EPS    = 0.05f;
    constexpr float DISTANCE_EPS = 0.001f;
    constexpr long  TIME_EPS_MS  = 1000;

    auto floatDiff = [](float a, float b) { return fabsf(a - b); };

    const bool speedChanged = floatDiff(speed.current, other.speed.current) >= SPEED_EPS ||
                              floatDiff(speed.max, other.speed.max) >= SPEED_EPS ||
                              floatDiff(speed.avg, other.speed.avg) >= SPEED_EPS;

    const bool distanceChanged = floatDiff(distance.trip, other.distance.trip) >= DISTANCE_EPS ||
                                 floatDiff(distance.total, other.distance.total) >= DISTANCE_EPS;

    const bool timeChanged = (abs((long)(time.elapsed - other.time.elapsed)) >= TIME_EPS_MS);

    return speedChanged || distanceChanged || timeChanged || (status != other.status) ||
           (fixMode != other.fixMode);
  }

private:
  void updateAccumulations(unsigned long dt) {
    if (status == Status::Paused) return;

    time.elapsed += dt;

    if (status == Status::Moving) {
      time.moving += dt;
      const float dDist = speed.current * (static_cast<float>(dt) / MS_TO_HOUR);
      distanceResidue += dDist;
      while (distanceResidue >= DISTANCE_UNIT_KM) {
        distance.trip += DISTANCE_UNIT_KM;
        distance.total += DISTANCE_UNIT_KM;
        distanceResidue -= DISTANCE_UNIT_KM;
      }
    }
  }

  void handleGnssUpdate(const GnssData &gnss) {
    fixMode            = (SpFixMode)gnss.navData.posFixMode;
    const float rawKmh = gnss.navData.velocity * 3.6f;
    const bool  fix    = hasFix(fixMode);
    const bool  moving = isMoving(fix, rawKmh);

    if (status != Status::Paused) { status = moving ? Status::Moving : Status::Stopped; }
    speed.current = (status == Status::Moving) ? rawKmh : 0.0f;
    if (speed.current > speed.max) speed.max = speed.current;
  }

  void handleGnssTimeout(unsigned long now) {
    if (now - lastUpdateTime > Config::Gnss::SIGNAL_TIMEOUT_MS) {
      if (status == Status::Moving) {
        status        = Status::Stopped;
        speed.current = 0.0f;
      }
    }
  }
};

inline SaveData::SaveData(const TripState &state, float v)
    : magicNumber(SAVE_DATA_MAGIC_NUMBER), totalDistance(state.distance.total),
      tripDistance(state.distance.trip), movingTimeMs(state.time.moving), maxSpeed(state.speed.max),
      voltage(v), crc(0) {}
