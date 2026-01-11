#include <cstdio>
#include <iostream>

#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include "GNSS.h"
#include "Wire.h"

// --- Wire ---
TwoWire Wire;

void TwoWire::begin() {
  // Mock implementation
}

// --- Adafruit_GFX ---
Adafruit_GFX::Adafruit_GFX(int16_t w, int16_t h) {
  // Mock implementation
  (void)w;
  (void)h;
}

// --- Adafruit_SSD1306 ---
Adafruit_SSD1306::Adafruit_SSD1306(int16_t w, int16_t h, TwoWire *twi, int8_t rst_pin) : Adafruit_GFX(w, h) {
  (void)twi;
  (void)rst_pin;
}

bool Adafruit_SSD1306::begin(uint8_t switchvcc, uint8_t i2caddr, bool reset, bool periphBegin) {
  (void)switchvcc;
  (void)i2caddr;
  (void)reset;
  (void)periphBegin;
  return true; // Success
}

void Adafruit_SSD1306::display() {
  // implementation
}

void Adafruit_SSD1306::clearDisplay() {
  // implementation
}

void Adafruit_SSD1306::invertDisplay(bool i) {
  (void)i;
}

void Adafruit_SSD1306::dim(bool dim) {
  (void)dim;
}

void Adafruit_SSD1306::drawPixel(int16_t x, int16_t y, uint16_t color) {
  (void)x;
  (void)y;
  (void)color;
}

void Adafruit_SSD1306::setTextSize(uint8_t s) {
  (void)s;
}

void Adafruit_SSD1306::setTextColor(uint16_t c) {
  (void)c;
}

void Adafruit_SSD1306::setCursor(int16_t x, int16_t y) {
  (void)x;
  (void)y;
}

void Adafruit_SSD1306::print(const String &s) {
  // std::cout << "OLED print: " << s.c_str() << std::endl;
}

void Adafruit_SSD1306::print(const char *s) {
  // std::cout << "OLED print: " << s << std::endl;
}

void Adafruit_SSD1306::println(const String &s) {
  // std::cout << "OLED println: " << s.c_str() << std::endl;
}

void Adafruit_SSD1306::println(const char *s) {
  // std::cout << "OLED println: " << s << std::endl;
}

void Adafruit_SSD1306::getTextBounds(const String &str, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) {
  // Mock logic to return some reasonable bounds
  // Assume 6x8 chars for size 1
  *x1 = x;
  *y1 = y;
  *w  = str.length() * 6;
  *h  = 8;
}

// Adafruit_GFX implementations
void Adafruit_GFX::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
  (void)x0;
  (void)y0;
  (void)x1;
  (void)y1;
  (void)color;
  // Mock implementation
}
void Adafruit_GFX::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  (void)x;
  (void)y;
  (void)w;
  (void)h;
  (void)color;
  // Mock implementation
}
void Adafruit_GFX::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  (void)x;
  (void)y;
  (void)w;
  (void)h;
  (void)color;
  // Mock implementation
}
void Adafruit_GFX::drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
  (void)x0;
  (void)y0;
  (void)r;
  (void)color;
  // Mock implementation
}

// --- GNSS ---
SpNavTime SpGnss::mockTimeData     = {2023, 10, 1, 12, 30, 0, 0};
float     SpGnss::mockVelocityData = 5.5f;

int SpGnss::begin() {
  return 0;
}
int SpGnss::start(int mode) {
  (void)mode;
  return 0;
}
int SpGnss::stop() {
  return 0;
}
void SpGnss::select(int satelliteSystem) {
  (void)satelliteSystem;
}
bool SpGnss::waitUpdate(int timeout) {
  (void)timeout;
  return true;
}
void SpGnss::getNavData(SpNavData *navData) {
  if (navData) {
    navData->velocity      = mockVelocityData;
    navData->time          = mockTimeData;
    navData->posFixMode    = Fix3D;
    navData->numSatellites = 8;
  }
}
