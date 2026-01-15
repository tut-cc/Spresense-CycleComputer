#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

class OLED {
public:
  struct Rect {
    int16_t  x;
    int16_t  y;
    uint16_t w;
    uint16_t h;
  };

private:
  const int16_t    width;
  const int16_t    height;
  Adafruit_SSD1306 ssd1306;

public:
  static constexpr int WIDTH   = 128;
  static constexpr int HEIGHT  = 64;
  static constexpr int ADDRESS = 0x3C;

  OLED(int16_t w, int16_t h) : width(w), height(h), ssd1306(w, h, &Wire, -1) {}

  bool begin(uint8_t address) {
    if (!ssd1306.begin(SSD1306_SWITCHCAPVCC, address)) return false;
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

  Rect getTextBounds(const char *string) {
    Rect rect;
    ssd1306.getTextBounds(string, 0, 0, &rect.x, &rect.y, &rect.w, &rect.h);
    return rect;
  }

  int getWidth() const {
    return width;
  }

  int getHeight() const {
    return height;
  }
};
