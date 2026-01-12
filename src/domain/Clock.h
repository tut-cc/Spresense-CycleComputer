#pragma once

#include <GNSS.h>

class Clock {
public:
  struct Time {
    int hour   = 0;
    int minute = 0;
    int second = 0;
  };

  Clock(int timeOffset = 9, int validYearStart = 2025) : timeOffset(timeOffset), validYearStart(validYearStart) {}

  void update(const SpNavData &navData) {
    year        = navData.time.year;
    time.hour   = (navData.time.hour + timeOffset + 24) % 24;
    time.minute = navData.time.minute;
    time.second = navData.time.sec;
  }

  Time getTime() const {
    if (year < validYearStart) return Time();
    return time;
  }

private:
  Time time;
  int  year = 0;
  int  timeOffset;
  int  validYearStart;
};
