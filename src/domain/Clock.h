#pragma once

#include <GNSS.h>

constexpr int JST_OFFSET       = 9;
constexpr int VALID_YEAR_START = 2026;

struct Clock {
  uint8_t hour    = 0;
  uint8_t minute  = 0;
  uint8_t second  = 0;
  bool    isValid = false;

  Clock(const SpNavData &navData) {
    if (navData.time.year < VALID_YEAR_START) return;

    hour   = adjustTimeZone(navData.time.hour, JST_OFFSET);
    minute = navData.time.minute;
    second = navData.time.sec;
  }

private:
  static uint8_t adjustTimeZone(int hourUTC, int offset) {
    return (hourUTC + offset + 24) % 24;
  }
};
