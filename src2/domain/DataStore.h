#pragma once

#include "../Config.h"
#include "SaveData.h"
#include <EEPROM.h>
#include <cmath>

struct DataStore {
  static constexpr unsigned long SAVE_INTERVAL_MS       = Config::Storage::SAVE_INTERVAL_MS;
  static constexpr uint32_t      SAVE_DATA_MAGIC_NUMBER = 0xDEADBEEF;

  static inline SaveData load() {
    SaveData s;
    EEPROM.get(Config::Storage::EEPROM_ADDR, s);
    if (s.calculateCRC() == s.crc && s.magic == SAVE_DATA_MAGIC_NUMBER &&
        !std::isnan(s.totalDist) && s.totalDist >= 0)
      return s;
    return SaveData();
  }

  static inline void save(const SaveData &s) { EEPROM.put(Config::Storage::EEPROM_ADDR, s); }

  static inline void clear() {
    SaveData d;
    d.magic = 0;
    d.updateCRC();
    EEPROM.put(Config::Storage::EEPROM_ADDR, d);
  }
};
