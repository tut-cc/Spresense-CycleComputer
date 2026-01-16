#pragma once

#include <GNSS.h>

class Gnss {
private:
  SpGnss    gnss;
  SpNavData navData{};

public:
  enum class StartMode { COLD, HOT };

  Gnss() {}

  bool begin(StartMode mode = StartMode::COLD) {
    if (gnss.begin() != 0) return false;

    selectSatellites();

    const SpStartMode startType = (mode == StartMode::COLD) ? COLD_START : HOT_START;
    if (gnss.start(startType) != 0) return false;

    return true;
  }

  bool update() {
    if (gnss.waitUpdate(0) != 1) return false;
    gnss.getNavData(&navData);
    return true;
  }

  SpNavData getNavData() const {
    return navData;
  }

private:
  void selectSatellites() {
    gnss.select(GPS);
    gnss.select(GLONASS);
    gnss.select(GALILEO);
    gnss.select(QZ_L1CA);
    gnss.select(QZ_L1S);
  }
};
