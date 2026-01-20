#pragma once

#include <Arduino.h>
#include <stddef.h>

struct TripData;

struct SaveData {
  uint32_t      magic     = 0xDEADBEEF;
  float         totalDist = 0;
  float         tripDist  = 0;
  unsigned long moveTime  = 0;
  float         maxSpd    = 0;
  float         volt      = 0;
  uint32_t      crc       = 0;

  SaveData() { updateCRC(); }
  SaveData(const TripData &s, float v);

  void updateCRC() { crc = calculateCRC(); }

  uint32_t calculateCRC() const {
    uint32_t       c = 0xFFFFFFFF;
    const uint8_t *p = (const uint8_t *)this;
    for (size_t i = 0; i < offsetof(SaveData, crc); i++) {
      c ^= p[i];
      for (int j = 0; j < 8; j++) c = (c >> 1) ^ (c & 1 ? 0xEDB88320 : 0);
    }
    return ~c;
  }

  bool operator==(const SaveData &o) const {
    return totalDist == o.totalDist && tripDist == o.tripDist && moveTime == o.moveTime &&
           maxSpd == o.maxSpd && volt == o.volt;
  }
  bool operator!=(const SaveData &o) const { return !(*this == o); }
};
