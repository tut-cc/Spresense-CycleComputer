#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <Wire.h>

#include "../interfaces/IDisplay.h"

class OLEDDriver : public IDisplay {
   private:
    Adafruit_SSD1306 display;
    DisplayDataType currentType;
    String currentValue;

   public:
    OLEDDriver();
    void begin() override;
    void clear() override;
    void show(DisplayDataType type, const char* value) override;
    void update() override {};
};
