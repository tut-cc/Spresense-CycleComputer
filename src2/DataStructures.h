#pragma once

#include <GNSS.h>

// 更新状態を表すenum
enum class UpdateStatus {
  NoChange,   // 変更なし
  Updated,    // 更新あり
  ForceUpdate // 強制更新（ユーザー入力など）
};

// 1. GNSSから更新されるデータ（入力）
struct GnssData {
  SpNavData     navData;
  unsigned long timestamp;
  UpdateStatus  status;
};

// 2. Trip状態（計算用）
// Note: 既存のTrip::Stateと互換性を保つため、まずは別名で定義
struct TripStateData {
  enum class Status { Stopped, Moving, Paused };

  // リアルタイム値（保存不要）
  float         currentSpeed;
  Status        status;
  SpFixMode     fixMode;
  unsigned long totalElapsedMs;

  // 累積値（保存必要）
  float         maxSpeed;
  float         totalKm;
  float         tripDistance;
  unsigned long totalMovingMs;

  // 派生値（再計算可能）
  float avgSpeed;

  // メタデータ
  unsigned long lastUpdateTime;
  UpdateStatus  updateStatus;
};

// 3. 表示用データ（文字列化前）
struct DisplayData {
  enum class SubType { Duration, Distance, Clock };

  // ヘッダー情報
  SpFixMode   fixMode;
  const char *modeSpeedLabel; // "SPD", "AVG", "MAX"
  const char *modeTimeLabel;  // "Time", "Odo", "Clock"

  // メイン表示値（数値）
  float       mainValue; // 速度値
  const char *mainUnit;  // "km/h"

  // サブ表示値（型が異なる）
  SubType subType;
  union {
    unsigned long durationMs; // SPD_TIMモード用
    float         distanceKm; // AVG_ODOモード用
    struct {
      int hour;
      int minute;
    } clockTime; // MAX_CLKモード用
  } subValue;
  const char *subUnit;

  // UI状態
  bool shouldBlink; // Pausedの点滅制御

  // 更新状態
  UpdateStatus updateStatus;
};

// 4. 永続化データ（保存用）
struct PersistentData {
  float         totalDistance;
  float         tripDistance;
  unsigned long movingTimeMs;
  float         maxSpeed;
  float         voltage;

  // 更新状態
  UpdateStatus updateStatus;

  bool operator==(const PersistentData &other) const {
    return totalDistance == other.totalDistance && tripDistance == other.tripDistance &&
           movingTimeMs == other.movingTimeMs && maxSpeed == other.maxSpeed &&
           voltage == other.voltage;
  }

  bool operator!=(const PersistentData &other) const {
    return !(*this == other);
  }
};
