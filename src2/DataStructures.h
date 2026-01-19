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

  bool isUpdated() const {
    return status == UpdateStatus::Updated;
  }
};

// 2. Trip状態（計算用）
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

  void resetMeta() {
    updateStatus = UpdateStatus::NoChange;
  }

  void forceUpdate() {
    updateStatus = UpdateStatus::ForceUpdate;
  }

  bool isPaused() const {
    return status == Status::Paused;
  }
  bool isMoving() const {
    return status == Status::Moving;
  }
};

// Internal state for coordinate history
struct TripStateDataEx : public TripStateData {
  float lastLat      = 0.0f;
  float lastLon      = 0.0f;
  bool  hasLastCoord = false;

  void resetAll() {
    currentSpeed   = 0.0f;
    status         = Status::Stopped;
    totalElapsedMs = 0;
    maxSpeed       = 0.0f;
    totalKm        = 0.0f;
    tripDistance   = 0.0f;
    totalMovingMs  = 0;
    avgSpeed       = 0.0f;
    lastUpdateTime = 0;
    hasLastCoord   = false;
    forceUpdate();
  }

  void resetTrip() {
    currentSpeed   = 0.0f;
    status         = Status::Stopped;
    totalElapsedMs = 0;
    tripDistance   = 0.0f;
    totalMovingMs  = 0;
    avgSpeed       = 0.0f;
    forceUpdate();
  }

  void resetMaxSpeed() {
    maxSpeed = 0.0f;
    forceUpdate();
  }
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

#include <cstring>

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

// 5. 表示用文字列データ（描画直前） - ダブルバッファ用
struct DisplayFrame {
  struct Item {
    char value[16];
    char unit[16];

    Item() {
      memset(value, 0, sizeof(value));
      memset(unit, 0, sizeof(unit));
    }

    bool operator==(const Item &other) const {
      return strcmp(value, other.value) == 0 && strcmp(unit, other.unit) == 0;
    }
    bool operator!=(const Item &other) const {
      return !(*this == other);
    }
  };

  struct Header {
    char fixStatus[8];
    char modeSpeed[8];
    char modeTime[8];

    Header() {
      memset(fixStatus, 0, sizeof(fixStatus));
      memset(modeSpeed, 0, sizeof(modeSpeed));
      memset(modeTime, 0, sizeof(modeTime));
    }

    bool operator==(const Header &other) const {
      return strcmp(fixStatus, other.fixStatus) == 0 && strcmp(modeSpeed, other.modeSpeed) == 0 &&
             strcmp(modeTime, other.modeTime) == 0;
    }
    bool operator!=(const Header &other) const {
      return !(*this == other);
    }
  };

  Header header;
  Item   main;
  Item   sub;

  DisplayFrame() = default;

  bool operator==(const DisplayFrame &other) const {
    return header == other.header && main == other.main && sub == other.sub;
  }
  bool operator!=(const DisplayFrame &other) const {
    return !(*this == other);
  }
};
