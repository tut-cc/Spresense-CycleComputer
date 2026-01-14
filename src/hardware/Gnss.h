#pragma once

#include <GNSS.h>
#include <cstring>

class Gnss {
private:
  SpGnss    gnss;
  SpNavData navData;

public:
  Gnss() {
    memset(&navData, 0, sizeof(navData));
  }

  bool begin() {
    if (gnss.begin() != 0) return false;
    gnss.select(GPS);
    gnss.select(GLONASS);
    gnss.select(GALILEO);
    gnss.select(QZ_L1CA);
    gnss.select(QZ_L1S);
    if (gnss.start(COLD_START) != 0) return false;
    return true;
  }

  bool update() {
    if (gnss.waitUpdate(0) != 1) return false;
    gnss.getNavData(&navData);
    return true;
  }

  const SpNavData &getNavData() const {
    return navData;
  }
};
