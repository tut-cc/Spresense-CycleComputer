/*
 * ファイル: OLEDDriver.cpp
 * 説明: Spresense OLEDディスプレイ用のディスプレイドライバー。
 */

#include "OLEDDriver.h"
#include "../Config.h"
#include <Arduino.h>

OLEDDriver::OLEDDriver() : display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1) {
    currentType = (DisplayDataType)-1; // Invalid initial type
}

void OLEDDriver::begin() {
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;); // Don't proceed, loop forever
    }
    display.clearDisplay();
    display.display();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    
    // Initial content
    currentValue = "";
}

void OLEDDriver::clear() {
    display.clearDisplay();
    display.display();
    currentValue = "";
    currentType = (DisplayDataType)-1;
}

void OLEDDriver::show(DisplayDataType type, const char* value) {
    // Check if we need to update
    if (type == currentType && currentValue.equals(value)) {
        return;
    }

    currentType = type;
    currentValue = String(value);

    display.clearDisplay();

    // 1. Draw Title (Top Left)
    display.setTextSize(1);
    display.setCursor(0, 0);

    String unit = "";
    String title = "";

    switch (type) {
        case DISPLAY_DATA_SPEED:
            title = "SPEED";
            unit = "km/h";
            break;
        case DISPLAY_DATA_TIME:
            title = "TIME";
            break;
        case DISPLAY_DATA_MAX_SPEED:
            title = "MAX SPEED";
            unit = "km/h";
            break;
        case DISPLAY_DATA_DISTANCE:
            title = "DISTANCE";
            unit = "km";
            break;
        case DISPLAY_DATA_MOVING_TIME:
            title = "MOVING TIME";
            break;
        case DISPLAY_DATA_ELAPSED_TIME:
            title = "ELAPSED TIME";
            break;
        case DISPLAY_DATA_AVG_SPEED:
            title = "AVG SPEED";
            unit = "km/h";
            break;
        default:
            title = "UNKNOWN";
            break;
    }

    display.println(title);

    // 2. Draw Value (Center, Large)
    // Adjust text size based on string length to fit
    int len = currentValue.length();
    if (len < 5) {
        display.setTextSize(3);
    } else {
        display.setTextSize(2);
    }
    
    // Simple centering calculation (approximate)
    // char w ~6px*size, h ~8px*size
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(currentValue, 0, 0, &x1, &y1, &w, &h);
    
    int x = (OLED_WIDTH - w) / 2;
    int y = (OLED_HEIGHT - h) / 2 + 8; // shift down a bit below title

    display.setCursor(x, y);
    display.print(currentValue);

    // 3. Draw Unit (Bottom Right)
    if (unit.length() > 0) {
        display.setTextSize(1);
        display.getTextBounds(unit, 0, 0, &x1, &y1, &w, &h);
        display.setCursor(OLED_WIDTH - w - 4, OLED_HEIGHT - h - 2);
        display.print(unit);
    }

    display.display();
}
