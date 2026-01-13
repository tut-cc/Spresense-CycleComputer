#pragma once

#include <cstring>

struct Frame {
  char title[16];
  char value[32];
  char unit[16];
  char fixStatus[8];
  char satelliteCount[8];

  bool operator==(const Frame &other) const {
    return strcmp(title, other.title) == 0 && strcmp(value, other.value) == 0 && strcmp(unit, other.unit) == 0 &&
           strcmp(fixStatus, other.fixStatus) == 0 && strcmp(satelliteCount, other.satelliteCount) == 0;
  }
};
