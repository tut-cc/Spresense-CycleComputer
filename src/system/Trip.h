#pragma once

#include "Odometer.h"
#include "Speedometer.h"
#include "Stopwatch.h"

#include <GNSS.h>

namespace application {

class Trip {
private:
  Speedometer   speedometer;
  Odometer      odometer;
  Stopwatch     stopwatch;
  unsigned long lastUpdateTime;

public:
  Trip() : lastUpdateTime(0) {}

  void begin() {
    reset();
  }

  void update(const SpNavData &navData, unsigned long currentMillis) {
    speedometer.update(navData);

    if (lastUpdateTime == 0) {
      lastUpdateTime = currentMillis;
      stopwatch.update(navData, 0, currentMillis); // Initialize start time logic if needed
      return;
    }

    unsigned long dt = currentMillis - lastUpdateTime;
    lastUpdateTime   = currentMillis;

    odometer.update(navData, dt);
    stopwatch.update(navData, dt, currentMillis);
  }

  void reset() {
    speedometer.reset();
    odometer.reset();
    stopwatch.reset();
    lastUpdateTime = 0;
  }

  float getSpeedKmh() const {
    return speedometer.get();
  }

  float getMaxSpeedKmh() const {
    return speedometer.getMax();
  }

  float getAvgSpeedKmh() const {
    unsigned long movingTimeMs = stopwatch.getMovingTime();
    if (movingTimeMs == 0) return 0.0f;
    return (odometer.getDistance() / (movingTimeMs / 3600000.0f));
  }

  float getDistanceKm() const {
    return odometer.getDistance();
  }

  void getMovingTimeStr(char *buffer, size_t size) const {
    stopwatch.getMovingTimeStr(buffer, size);
  }

  void getElapsedTimeStr(char *buffer, size_t size) const {
    stopwatch.getElapsedTimeStr(buffer, size);
  }

  void getSpeedStr(char *buffer, size_t size) const {
    formatFloat(getSpeedKmh(), 4, 1, buffer, size);
  }

  void getMaxSpeedStr(char *buffer, size_t size) const {
    formatFloat(getMaxSpeedKmh(), 4, 1, buffer, size);
  }

  void getDistanceStr(char *buffer, size_t size) const {
    formatFloat(getDistanceKm(), 5, 2, buffer, size);
  }

  void getAvgSpeedStr(char *buffer, size_t size) const {
    formatFloat(getAvgSpeedKmh(), 4, 1, buffer, size);
  }

private:
  void formatFloat(float val, int width, int prec, char *buf, size_t size) const {
    char fmt[8];
    snprintf(fmt, sizeof(fmt), "%%%d.%df", width, prec); // e.g. "%4.1f"
    snprintf(buf, size, fmt, val);
  }
};

} // namespace application
