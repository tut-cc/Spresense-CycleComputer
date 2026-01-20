#pragma once

/**
 * @file DataStructures.h
 * @brief アプリケーション全体で使用するデータ構造の定義
 *
 * GNSS、トリップ情報、保存データ、表示フレームなど、
 * モジュール間でやり取りされる主要なデータ型を定義します。
 */

#include <GNSS.h>
#include <cstring>

/// 状態更新の種類を示す列挙型
enum class UpdateStatus {
  NoChange,   // 変更なし
  Updated,    // 通常の更新
  ForceUpdate // 強制更新（ボタン押下等）
};

/// 表示モードを示す列挙型
enum class Mode {
  SPD_TIM, // 現在速度 & 経過時間
  AVG_ODO, // 平均速度 & 総走行距離
  MAX_CLK  // 最高速度 & 現在時刻
};

/// GNSSから取得したデータを保持する構造体
struct GnssData {
  SpNavData     navData;
  unsigned long timestamp;
  UpdateStatus  status;

  bool isUpdated() const { return status == UpdateStatus::Updated; }
};

struct TripStateBase {
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

  Status    status;
  SpFixMode fixMode;

  Speed    speed;
  Distance distance;
  Time     time;

  unsigned long lastUpdateTime;
  UpdateStatus  updateStatus;

  void resetMeta() { updateStatus = UpdateStatus::NoChange; }
  void forceUpdate() { updateStatus = UpdateStatus::ForceUpdate; }
  bool isPaused() const { return status == Status::Paused; }
  bool isMoving() const { return status == Status::Moving; }
};

struct TripState : public TripStateBase {
  float distanceResidue = 0.0f;

  void resetAll() {
    speed.current   = 0.0f;
    status          = Status::Stopped;
    time.elapsed    = 0;
    speed.max       = 0.0f;
    distance.total  = 0.0f;
    distance.trip   = 0.0f;
    time.moving     = 0;
    speed.avg       = 0.0f;
    lastUpdateTime  = 0;
    distanceResidue = 0.0f;
    forceUpdate();
  }

  void resetTrip() {
    speed.current   = 0.0f;
    status          = Status::Stopped;
    time.elapsed    = 0;
    distance.trip   = 0.0f;
    time.moving     = 0;
    speed.avg       = 0.0f;
    distanceResidue = 0.0f;
    forceUpdate();
  }

  void resetMaxSpeed() {
    speed.max = 0.0f;
    forceUpdate();
  }
};

/// EEPROM保存データの有効性を検証するためのマジックナンバー
/// 初期化されていないEEPROMや破損データを検出するために使用
constexpr uint32_t SAVE_DATA_MAGIC_NUMBER = 0xDEADBEEF;

struct SaveData {
  uint32_t      magicNumber;
  float         totalDistance;
  float         tripDistance;
  unsigned long movingTimeMs;
  float         maxSpeed;
  float         voltage;
  UpdateStatus  updateStatus;
  uint32_t      crc;

  bool operator==(const SaveData &other) const {
    return magicNumber == other.magicNumber && totalDistance == other.totalDistance &&
           tripDistance == other.tripDistance && movingTimeMs == other.movingTimeMs &&
           maxSpeed == other.maxSpeed && voltage == other.voltage;
  }

  bool operator!=(const SaveData &other) const { return !(*this == other); }
};

struct DisplayFrame {
  struct Header {
    const char *fixStatus;
    const char *modeSpeed;
    const char *modeTime;

    Header() : fixStatus(""), modeSpeed(""), modeTime("") {}

    bool operator==(const Header &other) const {
      return fixStatus == other.fixStatus && modeSpeed == other.modeSpeed &&
             modeTime == other.modeTime;
    }

    bool operator!=(const Header &other) const { return !(*this == other); }
  };

  struct Item {
    char        value[16];
    const char *unit;

    Item() : unit("") { memset(value, 0, sizeof(value)); }

    bool operator==(const Item &other) const {
      return strcmp(value, other.value) == 0 && unit == other.unit;
    }

    bool operator!=(const Item &other) const { return !(*this == other); }
  };

  Header header;
  Item   main;
  Item   sub;

  DisplayFrame() = default;

  bool operator==(const DisplayFrame &other) const {
    return header == other.header && main == other.main && sub == other.sub;
  }

  bool operator!=(const DisplayFrame &other) const { return !(*this == other); }
};
