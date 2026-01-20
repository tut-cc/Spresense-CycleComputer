#pragma once

#include <GNSS.h>
#include <cstring>

enum class UpdateStatus { NoChange, Updated, ForceUpdate };
enum class Mode { SPD_TIM, AVG_ODO, MAX_CLK };

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
