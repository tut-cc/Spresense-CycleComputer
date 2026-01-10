#pragma once

#include <cstdint>
#include <string>

#include "Arduino.h"

class Adafruit_GFX {
   public:
    Adafruit_GFX(int16_t w, int16_t h);
    virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;
};
