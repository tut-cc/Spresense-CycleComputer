#pragma once

#include "../Config.h"
#include <GNSS.h>
#include <cstdio>

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

  void getTimeStr(char *buffer, size_t size) const {
    if (year < 2025) {
      snprintf(buffer, size, "??:??");
      return;
    }

    snprintf(buffer, size, "%02d:%02d", hour, minute);
  }
};

} // namespace domain
