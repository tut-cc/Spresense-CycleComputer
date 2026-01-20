#pragma once

#include "../Config.h"
#include "../hardware/OLED.h"
#include "DisplayFrame.h"

class Renderer {
private:
  OLED oled;

public:
  Renderer() = default;
  bool begin() { return oled.begin(); }

  void render(const DisplayFrame &f) {
    oled.clear();
    drawHeader(f.header);
    drawItem(f.main, 30, 3, 1, false); // Main Area
    drawItem(f.sub, 64, 2, 1, true);   // Sub Area
    oled.display();
  }

  void resetDisplay() {
    oled.clear();
    oled.setTextSize(1);
    const char *msg  = "RESETTING...";
    OLED::Rect  rect = oled.getTextBounds(msg);
    oled.setCursor((Config::Display::WIDTH - rect.w) / 2, (Config::Display::HEIGHT - rect.h) / 2);
    oled.print(msg);
    oled.display();
    delay(500);
    oled.restart();
  }

private:
  void drawHeader(const DisplayFrame::Header &h) {
    oled.setTextSize(1);
    oled.setTextColor(WHITE);
    oled.setCursor(0, 0);
    oled.print(h.fixStatus);
    OLED::Rect r = oled.getTextBounds(h.modeSpeed);
    oled.setCursor((Config::Display::WIDTH - r.w) / 2, 0);
    oled.print(h.modeSpeed);
    r = oled.getTextBounds(h.modeTime);
    oled.setCursor(Config::Display::WIDTH - r.w, 0);
    oled.print(h.modeTime);
    oled.drawLine(0, 10, Config::Display::WIDTH, 10, WHITE);
  }

  void drawItem(const DisplayFrame::Item &item, int16_t y, uint8_t vSize, uint8_t uSize, bool btm) {
    oled.setTextSize(vSize);
    OLED::Rect vR = oled.getTextBounds(item.value);
    int16_t    tW = vR.w;
    OLED::Rect uR = {0, 0, 0, 0};
    if (item.unit[0]) {
      oled.setTextSize(uSize);
      uR = oled.getTextBounds(item.unit);
      tW += 4 + uR.w;
    }

    int16_t x  = (Config::Display::WIDTH - tW) / 2;
    int16_t vY = btm ? (y - vR.h) : (y - vR.h / 2);
    int16_t uY = btm ? (y - uR.h) : (y + vR.h / 2 - uR.h);

    oled.setTextSize(vSize);
    oled.setCursor(x, vY);
    oled.print(item.value);
    if (item.unit[0]) {
      oled.setTextSize(uSize);
      oled.setCursor(x + vR.w + 4, uY);
      oled.print(item.unit);
    }
  }
};
