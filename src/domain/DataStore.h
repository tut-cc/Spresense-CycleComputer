#pragma once

#include "../Config.h"
#include "SaveData.h"
#include <EEPROM.h>
#include <cmath>

struct DataStore {
  static constexpr unsigned long SAVE_INTERVAL_MS = Config::Storage::SAVE_INTERVAL_MS;

  static inline SaveData load() {
    SaveData saveData;
    EEPROM.get(Config::Storage::EEPROM_ADDR, saveData);
    if (saveData.isValid() && !std::isnan(saveData.totalDistance) && saveData.totalDistance >= 0)
      return saveData;

    return SaveData();
  }

  static inline void save(const SaveData &saveData) {
    EEPROM.put(Config::Storage::EEPROM_ADDR, saveData);
  }

  static inline void clear() {
    SaveData emptyData;
    emptyData.magic = 0;
    emptyData.updateCRC();
    EEPROM.put(Config::Storage::EEPROM_ADDR, emptyData);
  }
};
