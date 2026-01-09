#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <Wire.h>

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

class OLEDDriver {
   private:
    Adafruit_SSD1306 display;
    DisplayDataType currentType;
    String currentValue;

   public:
    OLEDDriver();
    virtual ~OLEDDriver() {}
    virtual void begin();
    virtual void clear();
    virtual void show(DisplayDataType type, const char* value);
};
