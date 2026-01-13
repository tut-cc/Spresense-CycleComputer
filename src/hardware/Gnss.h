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
  virtual ~Gnss() {}

  Gnss(const Gnss &)            = delete;
  Gnss &operator=(const Gnss &) = delete;

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
};
