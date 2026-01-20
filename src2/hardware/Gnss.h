#pragma once

/**
 * @file Gnss.h
 * @brief GNSSモジュールのラッパークラス
 *
 * GPS/GLONASS/Galileo/QZSS衛星からの位置・速度情報を取得します。
 */

#include <GNSS.h>

class Gnss {
public:
  SpGnss    gnss;
  SpNavData navData{};

public:
  Gnss() {}

  bool begin() {
    if (gnss.begin() != 0) return false;
    selectSatellites();
    if (gnss.start(COLD_START) != 0) return false;
    return true;
  }

  bool update() {
    if (gnss.waitUpdate(0) != 1) return false;
    gnss.getNavData(&navData);
    return true;
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
