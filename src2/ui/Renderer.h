#pragma once

#include "../Config.h"
#include "DisplayFrame.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

class Renderer {
private:
  Adafruit_SSD1306 d;

  struct Bounds {
    int16_t  x, y;
    uint16_t w, h;
  };

  inline Bounds getBounds(const char *s) {
    Bounds b;
    d.getTextBounds(s, 0, 0, &b.x, &b.y, &b.w, &b.h);
    return b;
  }

public:
  Renderer() : d(Config::Display::WIDTH, Config::Display::HEIGHT, &Wire, -1) {}

  inline bool begin() {
    if (!d.begin(SSD1306_SWITCHCAPVCC, Config::Display::ADDRESS)) return false;
    d.clearDisplay();
    d.display();
    return true;
  }

  inline void render(const DisplayFrame &f) {
    d.clearDisplay();
    drawHeader(f.header);
    drawItem(f.main, 30, 3, 1, false);
    drawItem(f.sub, 64, 2, 1, true);
    d.display();
  }

  inline void resetDisplay() {
    d.clearDisplay();
    d.setTextSize(1);
    const char *msg = "RESETTING...";
    Bounds      b   = getBounds(msg);
    d.setCursor((Config::Display::WIDTH - b.w) / 2, (Config::Display::HEIGHT - b.h) / 2);
    d.print(msg);
    d.display();
    delay(500);
    begin();
  }

private:
  inline void drawHeader(const DisplayFrame::Header &h) {
    d.setTextSize(1);
    d.setTextColor(WHITE);
    d.setCursor(0, 0);
    d.print(h.fixStatus);
    Bounds b = getBounds(h.modeSpeed);
    d.setCursor((Config::Display::WIDTH - b.w) / 2, 0);
    d.print(h.modeSpeed);
    b = getBounds(h.modeTime);
    d.setCursor(Config::Display::WIDTH - b.w, 0);
    d.print(h.modeTime);
    d.drawLine(0, 10, Config::Display::WIDTH, 10, WHITE);
  }

  inline void drawItem(const DisplayFrame::Item &item, int16_t y, uint8_t vSize, uint8_t uSize,
                       bool btm) {
    d.setTextSize(vSize);
    Bounds  vB = getBounds(item.value);
    int16_t tW = vB.w;
    Bounds  uB = {0, 0, 0, 0};
    if (item.unit[0]) {
      d.setTextSize(uSize);
      uB = getBounds(item.unit);
      tW += 4 + uB.w;
    }

    int16_t x  = (Config::Display::WIDTH - tW) / 2;
    int16_t vY = btm ? (y - vB.h) : (y - vB.h / 2);
    int16_t uY = btm ? (y - uB.h) : (y + vB.h / 2 - uB.h);

    d.setTextSize(vSize);
    d.setCursor(x, vY);
    d.print(item.value);
    if (item.unit[0]) {
      d.setTextSize(uSize);
      d.setCursor(x + vB.w + 4, uY);
      d.print(item.unit);
    }
  }
};
