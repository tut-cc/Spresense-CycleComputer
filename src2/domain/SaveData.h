#pragma once

#include "TripData.h"
#include <Arduino.h>
#include <stddef.h>

struct SaveData {
public:
  // START: 保存データ
  float         totalDistance = 0;
  unsigned long movingTime    = 0;
  float         maxSpeed      = 0;
  float         voltage       = 0;
  uint32_t      magic         = MAGIC_NUMBER;
  uint32_t      crc           = 0; // crcは必ずデータの最後尾に配置する
  // END: 保存データ

private:
  static constexpr uint32_t MAGIC_NUMBER = 0xC001BABE;

public:
  SaveData() { updateCRC(); }
  SaveData(const TripData &tripData, float batteryVoltage)
      : totalDistance(tripData.distance), movingTime(tripData.time.moving),
        maxSpeed(tripData.speed.max), voltage(batteryVoltage) {
    updateCRC();
  }

  TripData toTripData() const { return TripData(totalDistance, movingTime, maxSpeed); }

  void updateCRC() { crc = calculateCRC(); }

  bool isValid() const {
    const bool magicMatches = magic == MAGIC_NUMBER;
    const bool crcMatches   = calculateCRC() == crc;
    return magicMatches && crcMatches;
  }

  bool operator==(const SaveData &other) const {
    const bool totalDistanceEqual = totalDistance == other.totalDistance;
    const bool movingTimeEqual    = movingTime == other.movingTime;
    const bool maxSpeedEqual      = maxSpeed == other.maxSpeed;
    const bool voltageEqual       = voltage == other.voltage;
    return totalDistanceEqual && movingTimeEqual && maxSpeedEqual && voltageEqual;
  }
  bool operator!=(const SaveData &other) const { return !(*this == other); }

private:
  uint32_t calculateCRC() const {
    uint32_t       checksum    = 0xFFFFFFFF;
    const uint8_t *dataPointer = (const uint8_t *)this;
    for (size_t byteIndex = 0; byteIndex < offsetof(SaveData, crc); byteIndex++) {
      checksum ^= dataPointer[byteIndex];
      for (int bitIndex = 0; bitIndex < 8; bitIndex++)
        checksum = (checksum >> 1) ^ (checksum & 1 ? 0xEDB88320 : 0);
    }
    return ~checksum;
  }
};
