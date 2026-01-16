#pragma once
#include <cstring>
#include <stdint.h>

struct EEPROMClass {
  template <typename T> T &get(int idx, T &t) {
    // Return zeroed out data
    std::memset(&t, 0, sizeof(T));
    return t;
  }

  template <typename T> const T &put(int idx, const T &t) {
    return t;
  }
};

extern EEPROMClass EEPROM;
