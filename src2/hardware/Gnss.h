#pragma once

#include <GNSS.h>

class Gnss {
public:
  SpGnss    gnss;
  SpNavData navData{};

  bool begin() {
    if (gnss.begin() != 0) return false;
    gnss.select(GPS);
    gnss.select(GLONASS);
    gnss.select(QZ_L1CA);
    return gnss.start(COLD_START) == 0;
  }

  bool update() {
    if (gnss.waitUpdate(0) != 1) return false;
    gnss.getNavData(&navData);
    return true;
  }
};
