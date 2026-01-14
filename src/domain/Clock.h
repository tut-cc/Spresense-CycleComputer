#pragma once

#include <GNSS.h>

#include "../Config.h"

class Clock {
public:
  struct Time {
    int hour   = 0;
    int minute = 0;
    int second = 0;
  };

private:
  Time time;
  int  year = 0;

public:
  void update(const SpNavData &navData) {
    year        = navData.time.year;
    time.hour   = (navData.time.hour + Config::Time::JST_OFFSET + 24) % 24;
    time.minute = navData.time.minute;
    time.second = navData.time.sec;
  }

  Time getTime() const {
    if (year < Config::Time::VALID_YEAR_START) return Time();
    return time;
  }
};
