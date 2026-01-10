#include "DisplayData.h"

namespace application {

DisplayMetadata getDisplayMetadata(DisplayDataType type) {
  switch (type) {
  case DisplayDataType::SPEED:
    return {"SPEED", "km/h"};
  case DisplayDataType::MAX_SPEED:
    return {"MAX", "km/h"};
  case DisplayDataType::AVG_SPEED:
    return {"AVG", "km/h"};
  case DisplayDataType::TIME:
    return {"TIME", ""};
  case DisplayDataType::MOVING_TIME:
    return {"TRIP T", ""};
  case DisplayDataType::ELAPSED_TIME:
    return {"TIME", ""};
  case DisplayDataType::DISTANCE:
    return {"DIST", "km"};
  default:
    return {"", ""};
  }
}

} // namespace application
