#pragma once

#include "../DataStructures.h"
#include <EEPROM.h>
#include <math.h>
#include <stddef.h>

// 保存用設定
constexpr uint32_t      CRC_POLY     = 0xEDB88320;
constexpr uint32_t      MAGIC_NUMBER = 0xDEADBEEF;
constexpr float         MAX_VALID_KM = 1000000.0f; // 100万km
constexpr unsigned long EEPROM_ADDR  = 0;

class DataStore {
public:
  static constexpr float SAVE_INTERVAL_MS = 30000.0f;

private:
  struct SaveData {
    uint32_t       magicNumber;
    PersistentData data;
    uint32_t       crc;
  };

  SaveData lastSavedData;

public:
  PersistentData load() {
    SaveData savedData;
    EEPROM.get(EEPROM_ADDR, savedData);

    const uint32_t calculatedCrc = calculateDataCRC(savedData);

    if (isValid(savedData, calculatedCrc)) {
      lastSavedData = savedData;
      return savedData.data;
    }

    // デフォルト値
    PersistentData defaultData;
    defaultData.totalDistance = 0.0f;
    defaultData.tripDistance  = 0.0f;
    defaultData.movingTimeMs  = 0;
    defaultData.maxSpeed      = 0.0f;
    defaultData.voltage       = 0.0f;
    defaultData.updateStatus  = UpdateStatus::NoChange;

    lastSavedData.magicNumber = MAGIC_NUMBER;
    lastSavedData.data        = defaultData;
    lastSavedData.crc         = calculateDataCRC(lastSavedData);

    return defaultData;
  }

  void save(const PersistentData &currentData) {
    const bool isMagicValid = (lastSavedData.magicNumber == MAGIC_NUMBER);
    // 比較 (operator== を使用)
    if (isMagicValid && lastSavedData.data == currentData) return;

    SaveData saveData;
    saveData.magicNumber = MAGIC_NUMBER;
    saveData.data        = currentData;
    saveData.crc         = calculateDataCRC(saveData);

    // 書き込む前に一旦Magicを無効化（書き込み失敗検知用 - 既存仕様踏襲）
    uint32_t  invalidMagic = 0;
    const int magicAddr    = EEPROM_ADDR + offsetof(SaveData, magicNumber);
    EEPROM.put(magicAddr, invalidMagic);
    EEPROM.put(EEPROM_ADDR, saveData);

    lastSavedData = saveData;
  }

  void clear() {
    const int magicAddr = EEPROM_ADDR + offsetof(SaveData, magicNumber);
    EEPROM.put(magicAddr, (uint32_t)0);

    PersistentData cleanData;
    cleanData.totalDistance = 0.0f;
    cleanData.tripDistance  = 0.0f;
    cleanData.movingTimeMs  = 0;
    cleanData.maxSpeed      = 0.0f;
    cleanData.voltage       = 0.0f;
    cleanData.updateStatus  = UpdateStatus::NoChange;

    SaveData saveData;
    saveData.magicNumber = MAGIC_NUMBER;
    saveData.data        = cleanData;
    saveData.crc         = calculateDataCRC(saveData);

    EEPROM.put(EEPROM_ADDR, saveData);
    lastSavedData = saveData;
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
    if (data.magicNumber != MAGIC_NUMBER) return false;
    if (isnan(data.data.totalDistance)) return false;
    if (data.data.totalDistance < 0.0f) return false;
    if (MAX_VALID_KM < data.data.totalDistance) return false;
    return true;
  }
};
