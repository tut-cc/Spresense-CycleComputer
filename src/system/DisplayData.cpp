#include "DisplayData.h"

namespace application {

DisplayMetadata getDisplayMetadata(DisplayDataType type) {
  switch (type) {
  case DisplayDataType::SPEED:
    return {"SPEED", "km/h"};
  case DisplayDataType::MAX_SPEED:
    return {"MAX SPEED", "km/h"};
  case DisplayDataType::AVG_SPEED:
    return {"AVG SPEED", "km/h"};
  case DisplayDataType::TIME:
    return {"TIME", ""};
  case DisplayDataType::MOVING_TIME:
    return {"TRIP TIME", ""};
  case DisplayDataType::ELAPSED_TIME:
    return {"ELAPSED TIME", ""};
  case DisplayDataType::DISTANCE:
    return {"DISTANCE", "km"};
  default:
    return {"", ""};
  }
}

} // namespace application
