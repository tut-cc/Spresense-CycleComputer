#pragma once

#include <stdint.h>

enum class FixType { NoFix, Fix2D, Fix3D };

struct NavTime {
  int     year   = 0;
  uint8_t month  = 0;
  uint8_t day    = 0;
  uint8_t hour   = 0;
  uint8_t minute = 0;
  uint8_t second = 0;
};

struct NavData {
  double  latitude  = 0.0;
  double  longitude = 0.0;
  float   velocity  = 0.0f; // m/s
  NavTime time;
  FixType fixType = FixType::NoFix;
};
