#pragma once

#include <Arduino.h>

namespace application {

class TripManager {
private:
  float         distanceKm;
  float         maxSpeedKmh;
  unsigned long lastUpdateTime;

public:
  TripManager();
  void begin();
  void update(float currentSpeedKmh, unsigned long currentMillis);
  void reset();

  float getDistanceKm() const;
  float getMaxSpeedKmh() const;
  float getAvgSpeedKmh() const;
  void  getMovingTimeStr(char *buffer, size_t size) const;
  void  getElapsedTimeStr(char *buffer, size_t size) const;
  void  msToTimeStr(unsigned long ms, char *buffer, size_t size) const;

private:
  unsigned long movingTimeMs;
  unsigned long startTimeMs;
};

} // namespace application
