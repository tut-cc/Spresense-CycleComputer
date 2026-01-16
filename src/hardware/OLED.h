#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

constexpr int WIDTH   = 128;
constexpr int HEIGHT  = 64;
constexpr int ADDRESS = 0x3C;

class OLED {
public:
  struct Rect {
    int16_t  x;
    int16_t  y;
    uint16_t w;
    uint16_t h;
  };

private:
  Adafruit_SSD1306 ssd1306;

public:
  OLED() : ssd1306(WIDTH, HEIGHT, &Wire, -1) {}

  bool begin(uint8_t address) {
    if (!ssd1306.begin(SSD1306_SWITCHCAPVCC, address)) return false;
    ssd1306.clearDisplay();
    ssd1306.display();
    return true;
  }

  void restart() {
    begin(ADDRESS);
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
    return WIDTH;
  }

  int getHeight() const {
    return HEIGHT;
  }
};
