#pragma once
#include <cstring>
#include <stdint.h>

struct EEPROMClass {
  uint8_t buffer[1024];

  uint32_t writeCount = 0;

  EEPROMClass() {
    std::memset(buffer, 0, sizeof(buffer));
    writeCount = 0;
  }

  void clearWriteCount() {
    writeCount = 0;
  }

  template <typename T> T &get(int idx, T &t) {
    if (idx + sizeof(T) <= sizeof(buffer)) {
      std::memcpy(&t, &buffer[idx], sizeof(T));
    } else {
      std::memset(&t, 0, sizeof(T));
    }
    return t;
  }

  template <typename T> const T &put(int idx, const T &t) {
    if (idx + sizeof(T) <= sizeof(buffer)) {
      std::memcpy(&buffer[idx], &t, sizeof(T));
      writeCount++;
    }
    return t;
  }
};

extern EEPROMClass EEPROM;
