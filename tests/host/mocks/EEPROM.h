#pragma once
#include <cstring>
#include <stdint.h>

struct EEPROMClass {
  uint8_t buffer[1024];

  EEPROMClass() {
    std::memset(buffer, 0, sizeof(buffer));
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
    if (idx + sizeof(T) <= sizeof(buffer)) { std::memcpy(&buffer[idx], &t, sizeof(T)); }
    return t;
  }
};

extern EEPROMClass EEPROM;
