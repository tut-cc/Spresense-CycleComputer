#pragma once

#include "../Config.h"
#include "TripState.h"
#include <EEPROM.h>

class DataStore {
public:
  static constexpr unsigned long SAVE_INTERVAL_MS       = Config::Storage::SAVE_INTERVAL_MS;
  static constexpr uint32_t      SAVE_DATA_MAGIC_NUMBER = 0xDEADBEEF;

  SaveData load() {
    SaveData s;
    EEPROM.get(Config::Storage::EEPROM_ADDR, s);
    uint32_t c = calculateCRC(s);
    if (c == s.crc && s.magic == SAVE_DATA_MAGIC_NUMBER && !isnan(s.totalDist) && s.totalDist >= 0)
      return s;
    SaveData d;
    d.crc = calculateCRC(d);
    return d;
  }

  void save(const SaveData &s) {
    SaveData n = s;
    n.crc      = calculateCRC(n);
    EEPROM.put(Config::Storage::EEPROM_ADDR, n);
  }

  void clear() {
    SaveData d;
    d.magic = 0;
    d.crc   = calculateCRC(d);
    EEPROM.put(Config::Storage::EEPROM_ADDR, d);
  }

private:
  uint32_t calculateCRC(const SaveData &d) {
    uint32_t       crc = 0xFFFFFFFF;
    const uint8_t *p   = (const uint8_t *)&d;
    for (size_t i = 0; i < offsetof(SaveData, crc); i++) {
      crc ^= p[i];
      for (int j = 0; j < 8; j++) crc = (crc >> 1) ^ (crc & 1 ? 0xEDB88320 : 0);
    }
    return ~crc;
  }
};
