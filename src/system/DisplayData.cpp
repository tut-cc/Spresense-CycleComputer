#include "DisplayData.h"

DisplayMetadata getDisplayMetadata(DisplayDataType type) {
    switch (type) {
        case DisplayDataType::SPEED:
            return {"SPEED", "km/h"};
        case DisplayDataType::TIME:
            return {"TIME", ""};
        case DisplayDataType::MAX_SPEED:
            return {"MAX SPEED", "km/h"};
        case DisplayDataType::DISTANCE:
            return {"DISTANCE", "km"};
        case DisplayDataType::MOVING_TIME:
            return {"MOVING TIME", ""};
        case DisplayDataType::ELAPSED_TIME:
            return {"ELAPSED TIME", ""};
        case DisplayDataType::AVG_SPEED:
            return {"AVG SPEED", "km/h"};
        default:
            return {"UNKNOWN", ""};
    }
}
