#pragma once

#include <Arduino.h>
#include <GNSS.h>
#include <cstdio>

namespace application {

class Stopwatch {
private:
  unsigned long movingTimeMs;
  unsigned long startTimeMs;

public:
  Stopwatch() : movingTimeMs(0), startTimeMs(0) {}

  void update(const SpNavData &navData, unsigned long dtMs, unsigned long currentMillis) {
    if (startTimeMs == 0) startTimeMs = currentMillis;

    float speedKmh = navData.velocity * 3.6f;
    if (navData.posFixMode == FixInvalid) speedKmh = 0.0f;
    bool isMoving = (speedKmh > 3.0f);

    if (isMoving) movingTimeMs += dtMs;
  }

  void reset() {
    movingTimeMs = 0;
    startTimeMs  = millis();
  }

  unsigned long getMovingTime() const {
    return movingTimeMs;
  }

  unsigned long getElapsedTime() const {
    if (startTimeMs == 0) return 0;
    return millis() - startTimeMs;
  }

  void getMovingTimeStr(char *buffer, size_t size) const {
    msToTimeStr(movingTimeMs, buffer, size);
  }

  void getElapsedTimeStr(char *buffer, size_t size) const {
    msToTimeStr(getElapsedTime(), buffer, size);
  }

private:
  void msToTimeStr(unsigned long ms, char *buffer, size_t size) const {
    unsigned long totalSeconds = ms / 1000;
    unsigned long hours        = totalSeconds / 3600;
    unsigned long minutes      = (totalSeconds % 3600) / 60;
    snprintf(buffer, size, "%02d:%02d", (int)hours, (int)minutes);
  }
};

} // namespace application
