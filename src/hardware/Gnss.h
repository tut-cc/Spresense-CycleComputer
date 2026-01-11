#pragma once

#include <GNSS.h>

namespace hardware {

class Gnss {
private:
  SpGnss    gnss;
  SpNavData navData;

  Gnss() {}
  Gnss(const Gnss &)            = delete;
  Gnss &operator=(const Gnss &) = delete;

public:
  static Gnss &getInstance() {
    static Gnss instance;
    return instance;
  }

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

  const SpNavData &getNavData() const {
    return navData;
  }
};

} // namespace hardware
