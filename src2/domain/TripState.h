#pragma once

#include "../Config.h"
#include <Arduino.h>
#include <GNSS.h>

struct GnssData {
  SpNavData navData;
  bool      updated;
};

struct TripState;
constexpr float MS_TO_HOUR = 3600000.0f;

struct SaveData {
  uint32_t      magic     = 0xDEADBEEF;
  float         totalDist = 0;
  float         tripDist  = 0;
  unsigned long moveTime  = 0;
  float         maxSpd    = 0;
  float         volt      = 0;
  uint32_t      crc       = 0;

  SaveData() = default;
  SaveData(const TripState &s, float v);
  bool operator==(const SaveData &o) const {
    return totalDist == o.totalDist && tripDist == o.tripDist && moveTime == o.moveTime &&
           maxSpd == o.maxSpd && volt == o.volt;
  }
  bool operator!=(const SaveData &o) const { return !(*this == o); }
};

struct TripState {
  enum class Status { Stopped, Moving, Paused };
  struct Speed {
    float current, max, avg;
  } speed = {0, 0, 0};
  struct Dist {
    float total, trip;
  } distance = {0, 0};
  struct Time {
    unsigned long elapsed, moving;
  } time = {0, 0};

  Status        status      = Status::Stopped;
  SpFixMode     fixMode     = FixInvalid;
  unsigned long lastUpdate  = 0;
  float         distResidue = 0.0f;

  TripState() = default;
  TripState(const SaveData &s) {
    distance.total = s.totalDist;
    distance.trip  = s.tripDist;
    time.moving    = s.moveTime;
    speed.max      = s.maxSpd;
  }

  TripState(const TripState &p, const GnssData &g, unsigned long now) : TripState(p) {
    if (lastUpdate == 0) {
      lastUpdate = now;
      return;
    }
    unsigned long dt = now - lastUpdate;
    if (status != Status::Paused) {
      time.elapsed += dt;
      if (status == Status::Moving) {
        time.moving += dt;
        distResidue += speed.current * (dt / MS_TO_HOUR);
        while (distResidue >= 0.001f) {
          distance.trip += 0.001f;
          distance.total += 0.001f;
          distResidue -= 0.001f;
        }
      }
    }
    if (g.updated) {
      fixMode   = (SpFixMode)g.navData.posFixMode;
      float raw = g.navData.velocity * 3.6f;
      bool  mv  = (fixMode >= 2) && (raw > Config::Gnss::MIN_MOVING_SPEED_KMH);
      if (status != Status::Paused) status = mv ? Status::Moving : Status::Stopped;
      speed.current = (status == Status::Moving) ? raw : 0.0f;
      if (speed.current > speed.max) speed.max = speed.current;
    } else if (now - lastUpdate > Config::Gnss::SIGNAL_TIMEOUT_MS) {
      if (status == Status::Moving) {
        status        = Status::Stopped;
        speed.current = 0.0f;
      }
    }
    lastUpdate = now;
  }

  bool isPaused() const { return status == Status::Paused; }
  void clearAllData() { *this = TripState(); }
  void clearTripData() {
    speed.current = speed.avg = 0;
    status                    = Status::Stopped;
    time.elapsed = time.moving = 0;
    distance.trip = distResidue = 0;
  }
  void resetMaxSpeed() { speed.max = 0; }
  void updateAverageSpeed() {
    speed.avg = (time.moving > 0) ? (distance.trip / (time.moving / MS_TO_HOUR)) : 0;
  }

  bool operator!=(const TripState &o) const {
    return fabsf(speed.current - o.speed.current) > 0.05f ||
           fabsf(distance.trip - o.distance.trip) > 0.001f ||
           (time.elapsed / 1000 != o.time.elapsed / 1000) || status != o.status ||
           fixMode != o.fixMode;
  }
};

inline SaveData::SaveData(const TripState &s, float v)
    : totalDist(s.distance.total), tripDist(s.distance.trip), moveTime(s.time.moving),
      maxSpd(s.speed.max), volt(v) {}
