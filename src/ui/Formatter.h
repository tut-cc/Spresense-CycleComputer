#pragma once

#include <cstdio>

#include "../domain/Clock.h"

class Formatter {
public:
  static void formatSpeed(float speedKmh, char *buffer, size_t size) {
    snprintf(buffer, size, "%4.1f", speedKmh);
  }

  static void formatDistance(float distanceKm, char *buffer, size_t size) {
    snprintf(buffer, size, "%5.2f", distanceKm);
  }

  static void formatTime(const Clock::Time time, char *buffer, size_t size) {
    snprintf(buffer, size, "%02d:%02d", time.hour, time.minute);
  }

  static void formatDuration(unsigned long millis, char *buffer, size_t size) {
    const unsigned long seconds = millis / 1000;
    const unsigned long h       = seconds / 3600;
    const unsigned long m       = (seconds % 3600) / 60;
    const unsigned long s       = seconds % 60;

    if (0 < h) snprintf(buffer, size, "%lu:%02lu:%02lu", h, m, s);
    else snprintf(buffer, size, "%02lu:%02lu", m, s);
  }
};
