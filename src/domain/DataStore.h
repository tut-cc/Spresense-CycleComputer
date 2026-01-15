#pragma once

#include <EEPROM.h>
#include <math.h>
#include <stddef.h>

struct AppData {
  float         totalDistance;
  float         tripDistance;
  unsigned long movingTimeMs;
};

class DataStore {
private:
  struct SaveData {
    float         totalDistance;
    float         tripDistance;
    unsigned long movingTimeMs;
    uint32_t      magic;
    uint32_t      crc;
  };

  SaveData lastSavedData;

  static constexpr uint32_t CRC_POLY     = 0xEDB88320;
  static constexpr uint32_t MAGIC_NUMBER = 0xCAFEBABE;

  static constexpr float         MAX_VALID_KM = 1000000.0f; // 100万km
  static constexpr unsigned long EEPROM_ADDR  = 0;

public:
  static constexpr float SAVE_INTERVAL_MS = 30000.0f;

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

  uint32_t calculateDataCRC(const SaveData &data) {
    return calcCRC32((const uint8_t *)&data, offsetof(SaveData, crc));
  }

  bool isValid(const SaveData &data, uint32_t calculatedCrc) const {
    if (calculatedCrc != data.crc) return false;
    if (data.magic != MAGIC_NUMBER) return false;
    if (isnan(data.totalDistance)) return false;
    if (data.totalDistance < 0.0f) return false;
    if (data.totalDistance > MAX_VALID_KM) return false;
    return true;
  }

public:
  AppData load() {
    SaveData savedData;
    EEPROM.get(EEPROM_ADDR, savedData);

    const uint32_t calculatedCrc = calculateDataCRC(savedData);

    if (!isValid(savedData, calculatedCrc)) {
      // Invalid data, reset to default
      savedData     = {0.0f, 0.0f, 0, MAGIC_NUMBER, 0};
      savedData.crc = calculateDataCRC(savedData);
    }

    lastSavedData = savedData;

    return {savedData.totalDistance, savedData.tripDistance, savedData.movingTimeMs};
  }

  void save(const AppData &currentAppData) {
    SaveData currentData;
    currentData.totalDistance = currentAppData.totalDistance;
    currentData.tripDistance  = currentAppData.tripDistance;
    currentData.movingTimeMs  = currentAppData.movingTimeMs;
    currentData.magic         = MAGIC_NUMBER;
    currentData.crc           = calculateDataCRC(currentData);

    if (currentData.totalDistance != lastSavedData.totalDistance ||
        currentData.tripDistance != lastSavedData.tripDistance ||
        currentData.movingTimeMs != lastSavedData.movingTimeMs) {

      // 1. Invalidate signature
      uint32_t  invalidMagic = 0;
      const int magicAddr    = EEPROM_ADDR + offsetof(SaveData, magic);
      EEPROM.put(magicAddr, invalidMagic);

      // 2. Write new data
      EEPROM.put(EEPROM_ADDR, currentData);

      lastSavedData = currentData;
    }
  }

  void clear() {
    // 1. Invalidate first
    const int magicAddr = EEPROM_ADDR + offsetof(SaveData, magic);
    EEPROM.put(magicAddr, (uint32_t)0);

    // 2. Write clean data
    SaveData cleanData = {0.0f, 0.0f, 0, MAGIC_NUMBER, 0};
    cleanData.crc      = calculateDataCRC(cleanData);
    EEPROM.put(EEPROM_ADDR, cleanData);

    lastSavedData = cleanData;
  }
};
