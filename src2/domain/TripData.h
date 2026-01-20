#pragma once

#include "../Config.h"
#include "SaveData.h"
#include <Arduino.h>
#include <GNSS.h>
#include <RTC.h>

struct Clock {
  inline void begin() { RTC.begin(); }

  inline void sync(const SpGnssTime &gt) {
    if (gt.year < Config::Time::MIN_VALID_YEAR) return;
    RtcTime rt(gt.year, gt.month, gt.day, gt.hour, gt.minute, gt.sec);
    RTC.setTime(rt);
  }

  inline SpGnssTime now() {
    RtcTime rt = RTC.getTime();
    return {(unsigned short)rt.year(),
            (unsigned char)rt.month(),
            (unsigned char)rt.day(),
            (unsigned char)rt.hour(),
            (unsigned char)rt.minute(),
            (unsigned char)rt.second(),
            0};
  }
};

struct GnssData {
  SpNavData navData;
  bool      updated;
};

struct TripData;
constexpr float MS_TO_HOUR = 3600000.0f;

struct TripData {
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
  Clock         clock;

  TripData() = default;

  TripData(const SaveData &s) {
    distance.total = s.totalDist;
    distance.trip  = s.tripDist;
    time.moving    = s.moveTime;
    speed.max      = s.maxSpd;
  }

  TripData(const TripData &p, const GnssData &g, unsigned long now) : TripData(p) {
    if (g.updated && g.navData.posFixMode >= 2) clock.sync(g.navData.time);

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
  void clearAllData() { *this = TripData(); }

  void clearAvgOdo() {
    status        = Status::Stopped;
    speed.current = speed.avg = 0;
    time.elapsed = time.moving = 0;
    distance.trip = distResidue = 0;
  }

  void clearMaxSpeed() { speed.max = 0; }

  void updateAverageSpeed() {
    speed.avg = (time.moving > 0) ? (distance.trip / (time.moving / MS_TO_HOUR)) : 0;
  }

  bool operator!=(const TripData &o) const {
    return fabsf(speed.current - o.speed.current) > 0.05f ||
           fabsf(distance.trip - o.distance.trip) > 0.001f ||
           (time.elapsed / 1000 != o.time.elapsed / 1000) || status != o.status ||
           fixMode != o.fixMode;
  }
};

inline SaveData::SaveData(const TripData &s, float v)
    : totalDist(s.distance.total), tripDist(s.distance.trip), moveTime(s.time.moving),
      maxSpd(s.speed.max), volt(v) {
  updateCRC();
}
