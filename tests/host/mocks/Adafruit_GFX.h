#pragma once

#include <cstdint>
#include <string>

#include "Arduino.h"

#define WHITE 1

class Adafruit_GFX {
public:
  Adafruit_GFX(int16_t w, int16_t h);
  virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;
  virtual void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
  virtual void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
  virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
  virtual void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
};
