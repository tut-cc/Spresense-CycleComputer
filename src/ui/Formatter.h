#pragma once

#include <cstdio>

class Formatter {
public:
  static void formatSpeed(float speedKmh, char *buffer, size_t size) {
    snprintf(buffer, size, "%4.1f", speedKmh);
  }

  static void formatDistance(float distanceKm, char *buffer, size_t size) {
    snprintf(buffer, size, "%5.2f", distanceKm);
  }

  static void formatTime(int hour, int minute, char *buffer, size_t size) {
    snprintf(buffer, size, "%02d:%02d", hour, minute);
  }

  static void formatDuration(unsigned long millis, char *buffer, size_t size) {
    unsigned long seconds = millis / 1000;
    unsigned long h       = seconds / 3600;
    unsigned long m       = (seconds % 3600) / 60;
    unsigned long s       = seconds % 60;

    if (h > 0) snprintf(buffer, size, "%lu:%02lu:%02lu", h, m, s);
    else snprintf(buffer, size, "%02lu:%02lu", m, s);
  }
};
