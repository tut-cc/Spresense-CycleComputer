#pragma once

#include "../Config.h"
#include <Arduino.h>
#include <GNSS.h>
#include <RTC.h>

struct Clock {
  inline void begin() { RTC.begin(); }

  inline void sync(const SpGnssTime &gnssTime) {
    if (gnssTime.year < Config::Time::MIN_VALID_YEAR) return;
    RtcTime rtcTime(gnssTime.year, gnssTime.month, gnssTime.day, gnssTime.hour, gnssTime.minute,
                    gnssTime.sec);
    RTC.setTime(rtcTime);
  }

  inline SpGnssTime now() {
    RtcTime rtcTime = RTC.getTime();
    return {(unsigned short)rtcTime.year(),
            (unsigned char)rtcTime.month(),
            (unsigned char)rtcTime.day(),
            (unsigned char)rtcTime.hour(),
            (unsigned char)rtcTime.minute(),
            (unsigned char)rtcTime.second(),
            0};
  }
};

struct GnssData {
  SpNavData navData;
  bool      updated;
};

constexpr float MS_TO_HOUR = 3600000.0f;

struct TripData {
  enum class ActivityState { Stopped, Moving };

  struct Speed {
    float current, max, avg;
  } speed = {0.0f, 0.0f, 0.0f};

  float distance = 0.0f;

  struct Time {
    unsigned long elapsed, moving;
  } time = {0, 0};

  ActivityState activityState    = ActivityState::Stopped;
  bool          timerPaused      = false;
  SpFixMode     fixMode          = FixInvalid;
  unsigned long lastUpdate       = 0;
  float         distResidue      = 0.0f;
  float         weightedSpeedSum = 0.0f; // Σ(speed × deltaTime) for avg calculation
  Clock         clock;

  TripData() = default;

  TripData(float totalDistance, unsigned long movingTime, float maxSpeed) {
    distance    = totalDistance;
    time.moving = movingTime;
    speed.max   = maxSpeed;
  }

  TripData(const TripData &previous, const GnssData &gnssData, unsigned long currentTime)
      : TripData(previous) {
    if (gnssData.updated && gnssData.navData.posFixMode >= static_cast<int>(SpFixMode::Fix2D)) {
      clock.sync(gnssData.navData.time);
    }

    if (lastUpdate == 0) {
      lastUpdate = currentTime;
      return;
    }

    unsigned long deltaTime = currentTime - lastUpdate;

    // GPS更新の処理を先に行い、移動判定を取得
    bool isMoving = false;
    if (gnssData.updated) {
      fixMode        = (SpFixMode)gnssData.navData.posFixMode;
      float rawSpeed = gnssData.navData.velocity * 3.6f;

      // EMAフィルタで速度を平滑化
      float smoothedSpeed = Config::Gnss::SPEED_SMOOTHING * rawSpeed +
                            (1.0f - Config::Gnss::SPEED_SMOOTHING) * speed.current;

      isMoving      = (fixMode >= static_cast<int>(SpFixMode::Fix2D));
      isMoving      = isMoving && (smoothedSpeed > Config::Gnss::MIN_MOVING_SPEED_KMH);
      speed.current = isMoving ? smoothedSpeed : 0.0f;
      speed.max     = max(speed.max, speed.current);
      activityState = isMoving ? ActivityState::Moving : ActivityState::Stopped;

      // 診断用ログ
      Serial.print("Sats:");
      Serial.print(gnssData.navData.numSatellites);
      Serial.print(" PDOP:");
      Serial.print(gnssData.navData.pdop);
      Serial.print(" Raw:");
      Serial.print(rawSpeed);
      Serial.print(" Smooth:");
      Serial.println(smoothedSpeed);
    } else if (currentTime - lastUpdate > Config::Gnss::SIGNAL_TIMEOUT_MS) {
      speed.current = 0.0f;
      activityState = ActivityState::Stopped;
    }

    // elapsed time は timerPaused でないときのみカウント（表示用タイマー）
    if (!timerPaused) time.elapsed += deltaTime;

    // moving time と距離は実際に動いていればカウント（Paused中も継続）
    if (isMoving) {
      time.moving += deltaTime;
      weightedSpeedSum += speed.current * deltaTime;
      speed.avg = weightedSpeedSum / time.moving;
      distResidue += speed.current * (deltaTime / MS_TO_HOUR);
      while (distResidue >= 0.001f) {
        distance += 0.001f;
        distResidue -= 0.001f;
      }
    }

    lastUpdate = currentTime;
  }

  bool isMoving() const { return activityState == ActivityState::Moving; }
  bool isPaused() const { return timerPaused; }
  void togglePause() { timerPaused = !timerPaused; }
  void clearAllData() { *this = TripData(); }
  void clearMaxSpeed() { speed.max = 0; }

  void clearAvgOdo() {
    activityState = ActivityState::Stopped;
    timerPaused   = false;
    speed.current = speed.avg = 0.0f;
    weightedSpeedSum          = 0.0f;
    time.elapsed = time.moving = 0;
    distResidue                = 0.0f;
  }

  bool operator!=(const TripData &other) const {
    const bool speedChanged    = fabsf(speed.current - other.speed.current) > 0.05f;
    const bool distanceChanged = fabsf(distance - other.distance) > 0.001f;
    const bool elapsedChanged  = time.elapsed != other.time.elapsed;
    const bool stateChanged =
        (activityState != other.activityState) || (timerPaused != other.timerPaused);
    const bool fixModeChanged = fixMode != other.fixMode;
    return speedChanged || distanceChanged || elapsedChanged || stateChanged || fixModeChanged;
  }
};
