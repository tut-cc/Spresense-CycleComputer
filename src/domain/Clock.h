#pragma once

#include "NavData.h"

class Clock {
public:
  struct Time {
    uint8_t hour   = 0;
    uint8_t minute = 0;
    uint8_t second = 0;
  };

private:
  Time time;
  int  year = 0;

public:
  void update(const NavData &navData) {
    year        = navData.time.year;
    time.hour   = adjustHour(navData.time.hour, JST_OFFSET);
    time.minute = navData.time.minute;
    time.second = navData.time.second;
  }

  bool isValid() const {
    return year >= VALID_YEAR_START;
  }

  Time getTime() const {
    if (!isValid()) return Time();
    return time;
  }

private:
  static constexpr int JST_OFFSET       = 9;
  static constexpr int VALID_YEAR_START = 2026;

  static uint8_t adjustHour(int hour_utc, int offset) {
    return (hour_utc + offset + 24) % 24;
  }
};
