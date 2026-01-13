#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#include "../Config.h"

class OLED {
private:
  Adafruit_SSD1306 ssd1306;

public:
  virtual ~OLED() {}
  OLED() : ssd1306(Config::OLED::WIDTH, Config::OLED::HEIGHT, &Wire, -1) {}

  OLED(const OLED &)            = delete;
  OLED &operator=(const OLED &) = delete;

  virtual void begin() {
    if (!ssd1306.begin(SSD1306_SWITCHCAPVCC, Config::OLED::ADDRESS))
      for (;;);
  }

  virtual void clear() {
    ssd1306.clearDisplay();
  }

  virtual void display() {
    ssd1306.display();
  }

  virtual void setTextSize(int size) {
    ssd1306.setTextSize(size);
  }

  virtual void setTextColor(int color) {
    ssd1306.setTextColor(color);
  }

  virtual void setCursor(int x, int y) {
    ssd1306.setCursor(x, y);
  }

  virtual void print(const char *text) {
    ssd1306.print(text);
  }

  virtual void drawLine(int x0, int y0, int x1, int y1, int color) {
    ssd1306.drawLine(x0, y0, x1, y1, color);
  }

  virtual void drawRect(int x, int y, int w, int h, int color) {
    ssd1306.drawRect(x, y, w, h, color);
  }

  virtual void fillRect(int x, int y, int w, int h, int color) {
    ssd1306.fillRect(x, y, w, h, color);
  }

  virtual void drawCircle(int x0, int y0, int r, int color) {
    ssd1306.drawCircle(x0, y0, r, color);
  }

  virtual void getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) {
    ssd1306.getTextBounds(string, x, y, x1, y1, w, h);
  }

  virtual int getWidth() const {
    return Config::OLED::WIDTH;
  }

  virtual int getHeight() const {
    return Config::OLED::HEIGHT;
  }
};
