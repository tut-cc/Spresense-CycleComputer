#pragma once

#include <GNSS.h>

class Clock {
public:
  struct Time {
    uint8_t hour    = 0;
    uint8_t minute  = 0;
    uint8_t second  = 0;
    bool    isValid = false;
  };

private:
  static constexpr int JST_OFFSET       = 9;
  static constexpr int VALID_YEAR_START = 2026;

  Time time;

public:
  void update(const SpNavData &navData) {
    time.isValid = (navData.time.year >= VALID_YEAR_START);
    time.hour    = adjustHour(navData.time.hour, JST_OFFSET);
    time.minute  = navData.time.minute;
    time.second  = navData.time.sec;
  }

  Time getTime() const {
    if (!time.isValid) return Time();
    return time;
  }

private:
  static uint8_t adjustHour(int hour_utc, int offset) {
    return (hour_utc + offset + 24) % 24;
  }
};
