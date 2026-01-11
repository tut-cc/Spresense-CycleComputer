#pragma once

#include <Arduino.h>
#include <GNSS.h>

#include "../Config.h"

namespace application {

class GPSManager {
private:
  SpGnss    gnss;
  SpNavData navData;

public:
  bool begin() {
    if (gnss.begin() != 0) return false;
    gnss.select(GPS);
    gnss.select(QZ_L1CA);
    if (gnss.start(COLD_START) != 0) return false;
    return true;
  }

  void update() {
    if (!gnss.waitUpdate(0)) return;
    gnss.getNavData(&navData);
  }

  float getSpeedKmh() const {
    if (!(navData.posFixMode == Fix2D || navData.posFixMode == Fix3D)) return 0.0f;
    if (navData.velocity < 0.1f) return 0.0f; // 測位誤差対策
    float speedKmh = navData.velocity * 3.6f;
    return speedKmh;
  }

  void getTimeJST(char *displayData, size_t size) const {
    if (!(navData.time.year >= 2020)) {
      snprintf(displayData, size, "??:??");
      return;
    }

    int hour   = (navData.time.hour + Config::Time::JST_OFFSET + 24) % 24;
    int minute = navData.time.minute;
    snprintf(displayData, size, "%02d:%02d", hour, minute);
  }
};

} // namespace application
