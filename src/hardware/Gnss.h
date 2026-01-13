#pragma once

#include <Arduino.h>
#include <GNSS.h>
#include <cstring>

class Gnss {
private:
  SpGnss    gnss;
  SpNavData navData;

  Gnss(const Gnss &)            = delete;
  Gnss &operator=(const Gnss &) = delete;

public:
  Gnss() {
    memset(&navData, 0, sizeof(navData));
  }
  virtual ~Gnss() {}

  virtual bool begin() {
    if (gnss.begin() != 0) return false;
    gnss.select(GPS);
    gnss.select(QZ_L1CA);
    if (gnss.start(COLD_START) != 0) return false;
    return true;
  }

  virtual void update() {
    if (!gnss.waitUpdate(0)) return;
    gnss.getNavData(&navData);
  }

  virtual const SpNavData &getNavData() const {
    return navData;
  }

  virtual bool isFixed() const {
    // 0: No fix, 1: 2D fix, 2: 3D fix
    return navData.posFixMode >= 1;
  }
};
