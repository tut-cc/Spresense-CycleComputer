#include "OLEDDriver.h"

#include <Arduino.h>

#include "../Config.h"

namespace drivers {

OLEDDriver::OLEDDriver(TwoWire &i2c) : display(Config::OLED::WIDTH, Config::OLED::HEIGHT, &i2c, -1), wire(i2c) {}

void OLEDDriver::begin() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, Config::OLED::ADDRESS)) { for (;;); }
  display.clearDisplay();
  display.display();
}

void OLEDDriver::show(application::DisplayDataType type, const char *value) {
  display.clearDisplay();

  drawHeader();
  application::DisplayMetadata meta = application::getDisplayMetadata(type);
  drawMainArea(meta.title.c_str(), value, meta.unit.c_str());
  drawFooter();

  display.display();
}

void OLEDDriver::drawHeader() {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("GNSS ON");

  drawSatelliteIcon(100, 0, satelliteCount);
  drawBatteryIcon(115, 0, batteryLevel);

  display.drawLine(0, 10, Config::OLED::WIDTH, 10, SSD1306_WHITE);
}

void OLEDDriver::drawFooter() {
  display.drawLine(0, Config::OLED::HEIGHT - 10, Config::OLED::WIDTH, Config::OLED::HEIGHT - 10, SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, Config::OLED::HEIGHT - 8);
  display.print("Ready"); // Placeholder for status
}

void OLEDDriver::drawMainArea(const char *title, const char *value, const char *unit) {
  // Title
  display.setTextSize(1);
  display.setCursor(0, 14);
  display.print(title);

  // Value (Large)
  display.setTextSize(2); // Make value bigger
  int16_t  x1, y1;
  uint16_t w, h;
  display.getTextBounds(value, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((Config::OLED::WIDTH - w) / 2, 28);
  display.print(value);

  // Unit
  if (strlen(unit) > 0) {
    display.setTextSize(1);
    display.setCursor(Config::OLED::WIDTH - 24, 45); // Bottom right of main area
    display.print(unit);
  }
}

void OLEDDriver::drawBatteryIcon(int x, int y, int percentage) {
  display.drawRect(x, y, 12, 6, SSD1306_WHITE);
  display.fillRect(x + 12, y + 2, 2, 2, SSD1306_WHITE); // Battery positive terminal

  int width = map(percentage, 0, 100, 0, 10);
  display.fillRect(x + 1, y + 1, width, 4, SSD1306_WHITE);
}

void OLEDDriver::drawSatelliteIcon(int x, int y, int count) {
  display.drawCircle(x + 3, y + 3, 2, SSD1306_WHITE); // Placeholder for satellite icon
}

} // namespace drivers
