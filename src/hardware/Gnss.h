#pragma once

#include <GNSS.h>

#include "../domain/NavData.h"

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

  NavData getNavData() const {
    NavData data;
    data.latitude    = navData.latitude;
    data.longitude   = navData.longitude;
    data.velocity    = navData.velocity;
    data.time.year   = navData.time.year;
    data.time.month  = navData.time.month;
    data.time.day    = navData.time.day;
    data.time.hour   = navData.time.hour;
    data.time.minute = navData.time.minute;
    data.time.second = navData.time.sec;

    switch (navData.posFixMode) {
    case Fix2D:
      data.fixType = FixType::Fix2D;
      break;
    case Fix3D:
      data.fixType = FixType::Fix3D;
      break;
    default:
      data.fixType = FixType::NoFix;
      break;
    }
    return data;
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
