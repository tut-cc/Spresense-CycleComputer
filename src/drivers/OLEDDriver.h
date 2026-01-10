#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <Wire.h>

#include "../system/DisplayData.h"

class OLEDDriver {
private:
  Adafruit_SSD1306 display;
  DisplayDataType currentType;
  String currentValue;

  void drawTitle(const String &title);
  void drawUnit(const String &unit);
  void drawValue(const String &value);

public:
  OLEDDriver();
  virtual ~OLEDDriver() {}
  virtual bool begin();
  virtual void clear();
  virtual void show(DisplayDataType type, const char *value);
};
