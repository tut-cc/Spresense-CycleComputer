#pragma once

#include <Arduino.h>

namespace application {

enum class DisplayDataType {
  SPEED,
  MAX_SPEED,
  AVG_SPEED,
  TIME,
  MOVING_TIME,
  ELAPSED_TIME,
  DISTANCE,
  INVALID,
};

struct DisplayMetadata {
  String title;
  String unit;
};

DisplayMetadata getDisplayMetadata(DisplayDataType type);

} // namespace application
