#pragma once

#include "../Config.h"
#include "DisplayFrame.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

class Renderer {
private:
  Adafruit_SSD1306 display;

  struct TextBounds {
    int16_t  x, y;
    uint16_t width, height;
  };

  inline TextBounds getTextBounds(const char *text) {
    TextBounds bounds;
    display.getTextBounds(text, 0, 0, &bounds.x, &bounds.y, &bounds.width, &bounds.height);
    return bounds;
  }

public:
  Renderer() : display(Config::Display::WIDTH, Config::Display::HEIGHT, &Wire, -1) {}

  inline bool begin() {
    if (!display.begin(SSD1306_SWITCHCAPVCC, Config::Display::ADDRESS)) return false;
    display.clearDisplay();
    display.display();
    return true;
  }

  inline void render(const DisplayFrame &frame) {
    display.clearDisplay();
    drawHeader(frame.header);
    drawItem(frame.main, 30, 3, 1, false);
    drawItem(frame.sub, 64, 2, 1, true);
    display.display();
  }

  inline void resetDisplay() {
    display.clearDisplay();
    display.setTextSize(1);
    const char *message = "RESETTING...";
    TextBounds  bounds  = getTextBounds(message);
    display.setCursor((Config::Display::WIDTH - bounds.width) / 2,
                      (Config::Display::HEIGHT - bounds.height) / 2);
    display.print(message);
    display.display();
    delay(500);
    begin();
  }

private:
  inline void drawHeader(const Header &header) {
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.print(header.fixStatus);
    TextBounds bounds = getTextBounds(header.modeSpeed);
    display.setCursor((Config::Display::WIDTH - bounds.width) / 2, 0);
    display.print(header.modeSpeed);
    bounds = getTextBounds(header.modeTime);
    display.setCursor(Config::Display::WIDTH - bounds.width, 0);
    display.print(header.modeTime);
    display.drawLine(0, 10, Config::Display::WIDTH, 10, WHITE);
  }

  inline void drawItem(const Item &item, int16_t yPosition, uint8_t valueTextSize,
                       uint8_t unitTextSize, bool alignBottom) {
    display.setTextSize(valueTextSize);
    const TextBounds valueBounds = getTextBounds(item.value);
    int16_t          totalWidth  = valueBounds.width;
    TextBounds       unitBounds  = {0, 0, 0, 0};
    if (item.unit[0]) {
      display.setTextSize(unitTextSize);
      unitBounds = getTextBounds(item.unit);
      totalWidth += 4 + unitBounds.width;
    }

    const int16_t xPosition = (Config::Display::WIDTH - totalWidth) / 2;
    const int16_t valueY =
        alignBottom ? (yPosition - valueBounds.height) : (yPosition - valueBounds.height / 2);
    const int16_t unitY = alignBottom ? (yPosition - unitBounds.height)
                                      : (yPosition + valueBounds.height / 2 - unitBounds.height);

    display.setTextSize(valueTextSize);
    display.setCursor(xPosition, valueY);
    display.print(item.value);
    if (item.unit[0]) {
      display.setTextSize(unitTextSize);
      display.setCursor(xPosition + valueBounds.width + 4, unitY);
      display.print(item.unit);
    }
  }
};
