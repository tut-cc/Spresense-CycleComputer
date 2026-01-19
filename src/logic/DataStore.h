#pragma once

#include <EEPROM.h>
#include <math.h>
#include <stddef.h>

struct AppData {
  float         totalDistance;
  float         tripDistance;
  unsigned long movingTimeMs;
  float         maxSpeed;
  float         voltage;

  bool operator==(const AppData &other) const {
    const bool isMainDataEqual = isDataEqual(other);
    const bool isVoltageEqual  = voltage == other.voltage;
    return isMainDataEqual && isVoltageEqual;
  }

  bool operator!=(const AppData &other) const {
    return !(*this == other);
  }

  bool isDataEqual(const AppData &other) const {
    const bool isTripDistanceEqual  = tripDistance == other.tripDistance;
    const bool isTotalDistanceEqual = totalDistance == other.totalDistance;
    const bool isMovingTimeEqual    = movingTimeMs == other.movingTimeMs;
    const bool isMaxSpeedEqual      = maxSpeed == other.maxSpeed;
    return isTripDistanceEqual && isTotalDistanceEqual && isMovingTimeEqual && isMaxSpeedEqual;
  }
};

constexpr uint32_t      CRC_POLY     = 0xEDB88320;
constexpr uint32_t      MAGIC_NUMBER = 0xDEADBEEF;
constexpr float         MAX_VALID_KM = 1000000.0f; // 100万km
constexpr unsigned long EEPROM_ADDR  = 0;

class DataStore {
public:
  static constexpr float SAVE_INTERVAL_MS = 30000.0f;

private:
  struct SaveData {
    uint32_t magicNumber;
    AppData  data;
    uint32_t crc;
  };

  SaveData lastSavedData;

public:
  AppData load() {
    SaveData savedData;
    EEPROM.get(EEPROM_ADDR, savedData);

    const uint32_t calculatedCrc = calculateDataCRC(savedData);

    if (isValid(savedData, calculatedCrc)) {
      lastSavedData = savedData;
      return savedData.data;
    }

    AppData defaultData       = {0.0, 0.0, 0, 0.0, 0.0};
    lastSavedData.magicNumber = MAGIC_NUMBER;
    lastSavedData.data        = defaultData;
    lastSavedData.crc         = calculateDataCRC(lastSavedData);

    return defaultData;
  }

  void save(const AppData &currentAppData) {
    const bool isMagicValid = (lastSavedData.magicNumber == MAGIC_NUMBER);
    const bool isDataEqual  = lastSavedData.data.isDataEqual(currentAppData);

    if (isMagicValid && isDataEqual) return;

    SaveData currentData;
    currentData.magicNumber = MAGIC_NUMBER;
    currentData.data        = currentAppData;
    currentData.crc         = calculateDataCRC(currentData);

    uint32_t  invalidMagic = 0;
    const int magicAddr    = EEPROM_ADDR + offsetof(SaveData, magicNumber);
    EEPROM.put(magicAddr, invalidMagic);
    EEPROM.put(EEPROM_ADDR, currentData);

    lastSavedData = currentData;
  }

  void clear() {
    const int magicAddr = EEPROM_ADDR + offsetof(SaveData, magicNumber);
    EEPROM.put(magicAddr, (uint32_t)0);

    AppData  cleanAppData = {0.0f, 0.0f, 0, 0.0f, 0.0f};
    SaveData cleanData;
    cleanData.magicNumber = MAGIC_NUMBER;
    cleanData.data        = cleanAppData;
    cleanData.crc         = calculateDataCRC(cleanData);

    EEPROM.put(EEPROM_ADDR, cleanData);

    lastSavedData = cleanData;
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
