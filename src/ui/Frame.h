#pragma once

#include <cstring>

struct Frame {
  char value[32];
  char unit[16];
  char fixStatus[8];
  char satelliteCount[8];
  char footerTime[16];
  char footerMode[16];

  bool operator==(const Frame &other) const {
    return strcmp(value, other.value) == 0 && strcmp(unit, other.unit) == 0 &&
           strcmp(fixStatus, other.fixStatus) == 0 &&
           strcmp(satelliteCount, other.satelliteCount) == 0 &&
           strcmp(footerTime, other.footerTime) == 0 && strcmp(footerMode, other.footerMode) == 0;
  }
};
