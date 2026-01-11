#pragma once

#include "../Config.h"
#include <GNSS.h>

namespace domain {

class Clock {
private:
  int year;
  int hour;
  int minute;

public:
  void update(const SpNavData &navData) {
    year   = navData.time.year;
    hour   = (navData.time.hour + Config::Time::JST_OFFSET + 24) % 24;
    minute = navData.time.minute;
  }

  void getTime(int &h, int &m, int &s) const {
    if (year < 2025) {
      h = 0;
      m = 0;
      s = 0;
      return;
    }
    h = hour;
    m = minute;
    s = 0; // Currently we don't track seconds
  }
};

} // namespace domain
