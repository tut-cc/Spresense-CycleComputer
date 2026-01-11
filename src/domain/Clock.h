#pragma once

#include <GNSS.h>

class Clock {
public:
  struct Time {
    int hour;
    int minute;
    int second;
  };

  Clock(int timeOffset = 9, int validYearStart = 2025) : timeOffset(timeOffset), validYearStart(validYearStart) {}

  void update(const SpNavData &navData) {
    year   = navData.time.year;
    hour   = (navData.time.hour + timeOffset + 24) % 24;
    minute = navData.time.minute;
  }

  Time getTime() const {
    if (year < validYearStart) { return {0, 0, 0}; }
    return {hour, minute, 0}; // Currently we don't track seconds
  }

private:
  int year   = 0;
  int hour   = 0;
  int minute = 0;
  int timeOffset;
  int validYearStart;
};
