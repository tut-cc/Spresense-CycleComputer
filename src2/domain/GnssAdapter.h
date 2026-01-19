#pragma once
/**
 * @file GnssAdapter.h
 * @brief GNSSモジュールからデータを収集するアダプター
 */

#include "../common/DataStructures.h"
#include "../hardware/Gnss.h"
#include <Arduino.h>

namespace GnssAdapter {

/**
 * @brief GNSSからデータを収集
 * @param gnss GNSSモジュールへの参照
 * @return GnssData 収集したGNSSデータ
 */
inline GnssData collect(Gnss &gnss) {
  GnssData data;
  data.status    = gnss.update() ? UpdateStatus::Updated : UpdateStatus::NoChange;
  data.navData   = gnss.getNavData();
  data.timestamp = millis();
  return data;
}

} // namespace GnssAdapter
