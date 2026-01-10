#include "OLEDDriver.h"

#include <Arduino.h>

#include "../Config.h"
#include "../system/Logger.h"

OLEDDriver::OLEDDriver() : display(Config::OLED::WIDTH, Config::OLED::HEIGHT, &Wire, -1) {
    currentType = DisplayDataType::INVALID;
}

bool OLEDDriver::begin() {
    if (!display.begin(SSD1306_SWITCHCAPVCC, Config::OLED::ADDRESS)) {
        LOG_ERROR("SSD1306 allocation failed");
        return false;
    }

    display.clearDisplay();
    display.display();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);

    currentValue = "";
    return true;
}

void OLEDDriver::clear() {
    display.clearDisplay();
    display.display();
    currentValue = "";
    currentType = DisplayDataType::INVALID;
}

void OLEDDriver::drawTitle(const String& title) {
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println(title);
}

void OLEDDriver::drawUnit(const String& unit) {
    if (unit.length() <= 0) return;

    int16_t x1, y1;
    uint16_t w, h;
    display.setTextSize(1);
    display.getTextBounds(unit, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(Config::OLED::WIDTH - w - 4, Config::OLED::HEIGHT - h - 2);
    display.print(unit);
}

void OLEDDriver::drawValue(const String& value) {
    int len = value.length();
    if (len < 5)
        display.setTextSize(3);
    else
        display.setTextSize(2);

    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(value, 0, 0, &x1, &y1, &w, &h);

    int x = (Config::OLED::WIDTH - w) / 2;
    int y = (Config::OLED::HEIGHT - h) / 2 + 8;  // shift down a bit below title

    display.setCursor(x, y);
    display.print(value);
}

void OLEDDriver::show(DisplayDataType type, const char* value) {
    if (type == currentType && currentValue.equals(value)) return;

    currentType = type;
    currentValue = String(value);

    display.clearDisplay();

    DisplayMetadata meta = getDisplayMetadata(type);
    drawTitle(meta.title);
    drawValue(currentValue);
    drawUnit(meta.unit);

    display.display();
}
