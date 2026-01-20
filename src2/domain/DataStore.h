#pragma once

/**
 * @file DataStore.h
 * @brief EEPROM への永続データ保存・読み込み機能
 *
 * トリップデータ（走行距離、時間、最高速度など）をEEPROMに保存し、
 * 電源OFF後も値を保持します。CRCチェックによりデータ破損を検出します。
 */

#include "../common/Config.h"
#include "TripState.h"
#include <EEPROM.h>
#include <math.h>
#include <stddef.h>

/// CRC32計算用の多項式定数 (IEEE 802.3 標準)
constexpr uint32_t CRC_POLY = 0xEDB88320;

class DataStore {
public:
  static constexpr unsigned long SAVE_INTERVAL_MS = Config::Storage::SAVE_INTERVAL_MS;

  SaveData load() {
    SaveData savedData;
    EEPROM.get(Config::Storage::EEPROM_ADDR, savedData);

    const uint32_t calculatedCrc = calculateDataCRC(savedData);

    if (isValid(savedData, calculatedCrc)) return savedData;

    SaveData defaultData;
    defaultData.crc = calculateDataCRC(defaultData);

    return defaultData;
  }

  void save(const SaveData &currentData) {
    SaveData nextData = currentData;
    nextData.crc      = calculateDataCRC(nextData);
    EEPROM.put(Config::Storage::EEPROM_ADDR, nextData);
  }

  void clear() {
    const int magicAddr = Config::Storage::EEPROM_ADDR + offsetof(SaveData, magicNumber);
    EEPROM.put(magicAddr, (uint32_t)0);

    SaveData cleanData;
    cleanData.crc = calculateDataCRC(cleanData);

    EEPROM.put(Config::Storage::EEPROM_ADDR, cleanData);
  }

private:
  static uint32_t calcCRC32(const uint8_t *data, size_t length) {
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < length; i++) {
      crc ^= data[i];
      for (int j = 0; j < 8; j++) {
        if (crc & 1) crc = (crc >> 1) ^ CRC_POLY;
        else crc >>= 1;
      }
    }
    return ~crc;
  }

  static uint32_t calculateDataCRC(const SaveData &data) {
    return calcCRC32((const uint8_t *)&data, offsetof(SaveData, crc));
  }

  static bool isValid(const SaveData &data, uint32_t calculatedCrc) {
    const bool crcValid    = (calculatedCrc == data.crc);
    const bool magicValid  = (data.magicNumber == SAVE_DATA_MAGIC_NUMBER);
    const bool notNaN      = !isnan(data.totalDistance);
    const bool notNegative = (data.totalDistance >= 0.0f);
    const bool withinRange = (data.totalDistance <= Config::Storage::MAX_VALID_DISTANCE_KM);

    return crcValid && magicValid && notNaN && notNegative && withinRange;
  }
};
