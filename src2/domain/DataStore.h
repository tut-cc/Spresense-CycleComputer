#pragma once

/**
 * @file DataStore.h
 * @brief EEPROM への永続データ保存・読み込み機能
 *
 * トリップデータ（走行距離、時間、最高速度など）をEEPROMに保存し、
 * 電源OFF後も値を保持します。CRCチェックによりデータ破損を検出します。
 */

#include "../common/Config.h"
#include "../common/DataStructures.h"
#include <EEPROM.h>
#include <math.h>
#include <stddef.h>

/// CRC32計算用の多項式定数 (IEEE 802.3 標準)
constexpr uint32_t CRC_POLY = 0xEDB88320;

class DataStore {
public:
  /// 自動保存間隔 (Config.hから参照)
  static constexpr unsigned long SAVE_INTERVAL_MS = Config::Storage::SAVE_INTERVAL_MS;
  SaveData                       load() {
    SaveData savedData;
    EEPROM.get(Config::Storage::EEPROM_ADDR, savedData);

    const uint32_t calculatedCrc = calculateDataCRC(savedData);

    if (isValid(savedData, calculatedCrc)) return savedData;

    SaveData defaultData;
    defaultData.magicNumber   = SAVE_DATA_MAGIC_NUMBER;
    defaultData.totalDistance = 0.0f;
    defaultData.tripDistance  = 0.0f;
    defaultData.movingTimeMs  = 0;
    defaultData.maxSpeed      = 0.0f;
    defaultData.voltage       = 0.0f;
    defaultData.updateStatus  = UpdateStatus::NoChange;
    defaultData.crc           = calculateDataCRC(defaultData);

    return defaultData;
  }

  void save(const SaveData &currentData) {
    SaveData nextData    = currentData;
    nextData.magicNumber = SAVE_DATA_MAGIC_NUMBER;
    nextData.crc         = calculateDataCRC(nextData);

    uint32_t  invalidMagic = 0;
    const int magicAddr    = Config::Storage::EEPROM_ADDR + offsetof(SaveData, magicNumber);
    EEPROM.put(magicAddr, invalidMagic);

    EEPROM.put(Config::Storage::EEPROM_ADDR, nextData);
  }

  void clear() {
    const int magicAddr = Config::Storage::EEPROM_ADDR + offsetof(SaveData, magicNumber);
    EEPROM.put(magicAddr, (uint32_t)0);

    SaveData cleanData;
    cleanData.magicNumber   = SAVE_DATA_MAGIC_NUMBER;
    cleanData.totalDistance = 0.0f;
    cleanData.tripDistance  = 0.0f;
    cleanData.movingTimeMs  = 0;
    cleanData.maxSpeed      = 0.0f;
    cleanData.voltage       = 0.0f;
    cleanData.updateStatus  = UpdateStatus::NoChange;
    cleanData.crc           = calculateDataCRC(cleanData);

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
    if (calculatedCrc != data.crc) return false;
    if (data.magicNumber != SAVE_DATA_MAGIC_NUMBER) return false;
    if (isnan(data.totalDistance)) return false;
    if (data.totalDistance < 0.0f) return false;
    if (Config::Storage::MAX_VALID_DISTANCE_KM < data.totalDistance) return false;
    return true;
  }
};
