#pragma once

#include "../Config.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

class OLED {
public:
  struct Rect {
    int16_t  x, y;
    uint16_t w, h;
  };

private:
  Adafruit_SSD1306 s;

public:
  OLED() : s(Config::Display::WIDTH, Config::Display::HEIGHT, &Wire, -1) {}

  bool begin() {
    if (!s.begin(SSD1306_SWITCHCAPVCC, Config::Display::ADDRESS)) return false;
    s.clearDisplay();
    s.display();
    return true;
  }

  void restart() { begin(); }
  void clear() { s.clearDisplay(); }
  void display() { s.display(); }
  void setTextSize(int sz) { s.setTextSize(sz); }
  void setTextColor(int c) { s.setTextColor(c); }
  void setCursor(int x, int y) { s.setCursor(x, y); }
  void print(const char *t) { s.print(t); }
  void drawLine(int x0, int y0, int x1, int y1, int c) { s.drawLine(x0, y0, x1, y1, c); }

  Rect getTextBounds(const char *str) {
    Rect r;
    s.getTextBounds(str, 0, 0, &r.x, &r.y, &r.w, &r.h);
    return r;
  }
};
