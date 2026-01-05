#pragma once

#include <Arduino.h>

enum class DisplayDataType {
    SPEED,
    TIME,
    MAX_SPEED,
    DISTANCE,
    MOVING_TIME,
    ELAPSED_TIME,
    AVG_SPEED,
    INVALID,
};

class IDisplay {
   public:
    virtual ~IDisplay() {}
    virtual void begin() = 0;
    virtual void clear() = 0;
    virtual void show(DisplayDataType type, const char* value) = 0;
    virtual void update() {}; // For background refresh tasks (e.g. multiplexing)
};
