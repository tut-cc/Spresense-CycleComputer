#pragma once

#include <GNSS.h>
#include <cstring>

enum class UpdateStatus { NoChange, Updated, ForceUpdate };
enum class Mode { SPD_TIM, AVG_ODO, MAX_CLK };

struct GnssData {
  SpNavData     navData;
  unsigned long timestamp;
  UpdateStatus  status;

  bool isUpdated() const {
    return status == UpdateStatus::Updated;
  }
};

struct TripStateData {
  enum class Status { Stopped, Moving, Paused };

  float         currentSpeed;
  Status        status;
  SpFixMode     fixMode;
  unsigned long totalElapsedMs;

  float         maxSpeed;
  float         totalKm;
  float         tripDistance;
  unsigned long totalMovingMs;
  float         avgSpeed;

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

struct TripStateDataEx : public TripStateData {
  float lastLat         = 0.0f;
  float lastLon         = 0.0f;
  bool  hasLastCoord    = false;
  float distanceResidue = 0.0f;

  void resetAll() {
    currentSpeed    = 0.0f;
    status          = Status::Stopped;
    totalElapsedMs  = 0;
    maxSpeed        = 0.0f;
    totalKm         = 0.0f;
    tripDistance    = 0.0f;
    totalMovingMs   = 0;
    avgSpeed        = 0.0f;
    lastUpdateTime  = 0;
    hasLastCoord    = false;
    distanceResidue = 0.0f;
    forceUpdate();
  }

  void resetTrip() {
    currentSpeed    = 0.0f;
    status          = Status::Stopped;
    totalElapsedMs  = 0;
    tripDistance    = 0.0f;
    totalMovingMs   = 0;
    avgSpeed        = 0.0f;
    distanceResidue = 0.0f;
    forceUpdate();
  }

  void resetMaxSpeed() {
    maxSpeed = 0.0f;
    forceUpdate();
  }
};

struct DisplayData {
  enum class SubType { Duration, Distance, Clock };

  SpFixMode   fixMode;
  const char *modeSpeedLabel; // "SPD", "AVG", "MAX"
  const char *modeTimeLabel;  // "Time", "Odo", "Clock"

  float       mainValue; // 速度値
  const char *mainUnit;  // "km/h"

  SubType subType;
  union {
    unsigned long durationMs; // SPD_TIMモード用
    float         distanceKm; // AVG_ODOモード用
    struct {
      int hour;
      int minute;
    } clockTime; // MAX_CLKモード用
  } subValue;

  const char  *subUnit;
  bool         shouldBlink;
  UpdateStatus updateStatus;
};

constexpr uint32_t SAVE_DATA_MAGIC_NUMBER = 0xDEADBEEF;

struct SaveData {
  uint32_t magicNumber;

  float         totalDistance;
  float         tripDistance;
  unsigned long movingTimeMs;
  float         maxSpeed;
  float         voltage;

  UpdateStatus updateStatus;

  uint32_t crc;

  bool operator==(const SaveData &other) const {
    return magicNumber == other.magicNumber && totalDistance == other.totalDistance &&
           tripDistance == other.tripDistance && movingTimeMs == other.movingTimeMs &&
           maxSpeed == other.maxSpeed && voltage == other.voltage;
  }

  bool operator!=(const SaveData &other) const {
    return !(*this == other);
  }
};

struct DisplayFrame {
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
