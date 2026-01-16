#pragma once

#include <GNSS.h>

struct Clock {
  uint8_t hour    = 0;
  uint8_t minute  = 0;
  uint8_t second  = 0;
  bool    isValid = false;

  Clock(const SpNavData &navData) {
    isValid = VALID_YEAR_START <= navData.time.year;
    if (!isValid) return;

    hour   = adjustTimeZone(navData.time.hour, JST_OFFSET);
    minute = navData.time.minute;
    second = navData.time.sec;
  }

  Clock() = default;

private:
  static constexpr int JST_OFFSET       = 9;
  static constexpr int VALID_YEAR_START = 2026;

  static uint8_t adjustTimeZone(int hourUTC, int offset) {
    return (hourUTC + offset + 24) % 24;
  }
};
