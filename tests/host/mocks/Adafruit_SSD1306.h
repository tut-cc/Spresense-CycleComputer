#pragma once

#include <cstdint>

#include "Adafruit_GFX.h"
#include "Wire.h"

#define SSD1306_SWITCHCAPVCC 0
#define SSD1306_WHITE 1

class Adafruit_SSD1306 : public Adafruit_GFX {
   public:
    Adafruit_SSD1306(int16_t w, int16_t h, TwoWire *twi = &Wire, int8_t rst_pin = -1);

    bool begin(uint8_t switchvcc = SSD1306_SWITCHCAPVCC, uint8_t i2caddr = 0, bool reset = true,
               bool periphBegin = true);
    void display();
    void clearDisplay();
    void invertDisplay(bool i);
    void dim(bool dim);
    void drawPixel(int16_t x, int16_t y, uint16_t color) override;

    void setTextSize(uint8_t s);
    void setTextColor(uint16_t c);
    void setCursor(int16_t x, int16_t y);
    void print(const String &s);
    void print(const char *s);
    void println(const String &s);
    void println(const char *s);

    void getTextBounds(const String &str, int16_t x, int16_t y, int16_t *x1, int16_t *y1,
                       uint16_t *w, uint16_t *h);
};
