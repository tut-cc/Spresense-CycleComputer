#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <Wire.h>

#include "../hal/interfaces/IDisplay.h"
#include "../system/DisplayData.h"

namespace drivers {

class OLEDDriver : public hal::IDisplay {
public:
  OLEDDriver(TwoWire &i2c);
  void begin() override;
  void show(application::DisplayDataType type, const char *value) override;

private:
  Adafruit_SSD1306 display;
  TwoWire         &wire;

  void drawHeader();
  void drawFooter();
  void drawMainArea(const char *title, const char *value, const char *unit);
  void drawBatteryIcon(int x, int y, int percentage);
  void drawSatelliteIcon(int x, int y, int count);

  int batteryLevel   = 85;
  int satelliteCount = 5;
};

} // namespace drivers
