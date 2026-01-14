#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#include "../Config.h"

class OLED {
private:
  Adafruit_SSD1306 ssd1306;

public:
  OLED() : ssd1306(Config::OLED::WIDTH, Config::OLED::HEIGHT, &Wire, -1) {}

  bool begin() {
    if (!ssd1306.begin(SSD1306_SWITCHCAPVCC, Config::OLED::ADDRESS)) return false;
    ssd1306.clearDisplay();
    ssd1306.display();
    return true;
  }

  void clear() {
    ssd1306.clearDisplay();
  }

  void display() {
    ssd1306.display();
  }

  void setTextSize(int size) {
    ssd1306.setTextSize(size);
  }

  void setTextColor(int color) {
    ssd1306.setTextColor(color);
  }

  void setCursor(int x, int y) {
    ssd1306.setCursor(x, y);
  }

  void print(const char *text) {
    ssd1306.print(text);
  }

  void drawLine(int x0, int y0, int x1, int y1, int color) {
    ssd1306.drawLine(x0, y0, x1, y1, color);
  }

  void drawRect(int x, int y, int w, int h, int color) {
    ssd1306.drawRect(x, y, w, h, color);
  }

  void fillRect(int x, int y, int w, int h, int color) {
    ssd1306.fillRect(x, y, w, h, color);
  }

  void drawCircle(int x0, int y0, int r, int color) {
    ssd1306.drawCircle(x0, y0, r, color);
  }

  void getTextBounds(char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w,
                     uint16_t *h) {
    ssd1306.getTextBounds(string, x, y, x1, y1, w, h);
  }

  int getWidth() const {
    return Config::OLED::WIDTH;
  }

  int getHeight() const {
    return Config::OLED::HEIGHT;
  }
};
