#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <Wire.h>

#include "../Config.h"
#include "../ui/GraphicsContext.h"

namespace drivers {

class OLED : public ui::GraphicsContext {
private:
  Adafruit_SSD1306 ssd1306;

public:
  OLED() : ssd1306(Config::OLED::WIDTH, Config::OLED::HEIGHT, &Wire, -1) {}

  void begin() {
    if (!ssd1306.begin(SSD1306_SWITCHCAPVCC, Config::OLED::ADDRESS))
      for (;;);
    ssd1306.clearDisplay();
    ssd1306.display();
  }

  // GraphicsContext implementation
  void clear() override {
    ssd1306.clearDisplay();
  }

  void display() override {
    ssd1306.display();
  }

  void setTextSize(int size) override {
    ssd1306.setTextSize(size);
  }

  void setTextColor(int color) override {
    ssd1306.setTextColor(color);
  }

  void setCursor(int x, int y) override {
    ssd1306.setCursor(x, y);
  }

  void print(const char *text) override {
    ssd1306.print(text);
  }

  void drawLine(int x0, int y0, int x1, int y1, int color) override {
    ssd1306.drawLine(x0, y0, x1, y1, color);
  }

  void drawRect(int x, int y, int w, int h, int color) override {
    ssd1306.drawRect(x, y, w, h, color);
  }

  void fillRect(int x, int y, int w, int h, int color) override {
    ssd1306.fillRect(x, y, w, h, color);
  }

  void drawCircle(int x0, int y0, int r, int color) override {
    ssd1306.drawCircle(x0, y0, r, color);
  }

  void getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) override {
    ssd1306.getTextBounds(string, x, y, x1, y1, w, h);
  }

  int getWidth() const override {
    return Config::OLED::WIDTH;
  }

  int getHeight() const override {
    return Config::OLED::HEIGHT;
  }
};

} // namespace drivers
