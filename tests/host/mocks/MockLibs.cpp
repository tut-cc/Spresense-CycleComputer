#include <algorithm>
#include <chrono>
#include <cstdio>
#include <thread>

#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include "DisplayLogger.h"
#include "GNSS.h"
#include "Wire.h"

// --- DisplayLogger ---
std::vector<DrawCall> DisplayLogger::calls;

// --- Wire ---
TwoWire Wire;

void TwoWire::begin() {
  // Mock implementation
}

// --- Adafruit_GFX ---
Adafruit_GFX::Adafruit_GFX(int16_t w, int16_t h) {
  (void)w;
  (void)h;
}

// --- Adafruit_SSD1306 ---
bool Adafruit_SSD1306::mockBeginResult = true;

Adafruit_SSD1306::Adafruit_SSD1306(int16_t w, int16_t h, TwoWire *twi, int8_t rst_pin)
    : Adafruit_GFX(w, h) {
  (void)twi;
  (void)rst_pin;
}

bool Adafruit_SSD1306::begin(uint8_t switchvcc, uint8_t i2caddr, bool reset, bool periphBegin) {
  (void)switchvcc;
  (void)i2caddr;
  (void)reset;
  (void)periphBegin;
  return mockBeginResult;
}

void Adafruit_SSD1306::display() {
  DisplayLogger::log({DrawCall::Type::Display, 0, 0, 0, 0, 0, 0, 0, ""});
}

void Adafruit_SSD1306::clearDisplay() {
  DisplayLogger::log({DrawCall::Type::Clear, 0, 0, 0, 0, 0, 0, 0, ""});
}

void Adafruit_SSD1306::invertDisplay(bool i) {
  (void)i;
}

void Adafruit_SSD1306::dim(bool dim) {
  (void)dim;
}

void Adafruit_SSD1306::drawPixel(int16_t x, int16_t y, uint16_t color) {
  DisplayLogger::log({DrawCall::Type::Pixel, x, y, 0, 0, (int16_t)color, 0, 0, ""});
}

void Adafruit_SSD1306::setTextSize(uint8_t s) {
  DisplayLogger::log({DrawCall::Type::TextSize, 0, 0, 0, 0, 0, (int16_t)s, 0, ""});
}

void Adafruit_SSD1306::setTextColor(uint16_t c) {
  DisplayLogger::log({DrawCall::Type::TextColor, 0, 0, 0, 0, (int16_t)c, 0, 0, ""});
}

void Adafruit_SSD1306::setCursor(int16_t x, int16_t y) {
  DisplayLogger::log({DrawCall::Type::Cursor, x, y, 0, 0, 0, 0, 0, ""});
}

void Adafruit_SSD1306::print(const String &s) {
  DisplayLogger::log({DrawCall::Type::Text, 0, 0, 0, 0, 0, 0, 0, s.c_str()});
}

void Adafruit_SSD1306::print(const char *s) {
  if (s) DisplayLogger::log({DrawCall::Type::Text, 0, 0, 0, 0, 0, 0, 0, s});
}

void Adafruit_SSD1306::println(const String &s) {
  DisplayLogger::log({DrawCall::Type::Text, 0, 0, 0, 0, 0, 0, 0, std::string(s.c_str()) + "\n"});
}

void Adafruit_SSD1306::println(const char *s) {
  if (s) DisplayLogger::log({DrawCall::Type::Text, 0, 0, 0, 0, 0, 0, 0, std::string(s) + "\n"});
}

void Adafruit_SSD1306::getTextBounds(const String &str, int16_t x, int16_t y, int16_t *x1,
                                     int16_t *y1, uint16_t *w, uint16_t *h) {
  *x1 = x;
  *y1 = y;
  *w  = str.length() * 6;
  *h  = 8;
}

// Adafruit_GFX implementations
void Adafruit_GFX::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
  DisplayLogger::log({DrawCall::Type::Line, x0, y0, x1, y1, (int16_t)color, 0, 0, ""});
}
void Adafruit_GFX::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  DisplayLogger::log(
      {DrawCall::Type::Rect, x, y, (int16_t)(x + w), (int16_t)(y + h), (int16_t)color, 0, 0, ""});
}
void Adafruit_GFX::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  DisplayLogger::log({DrawCall::Type::FillRect, x, y, (int16_t)(x + w), (int16_t)(y + h),
                      (int16_t)color, 0, 0, ""});
}
void Adafruit_GFX::drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
  DisplayLogger::log({DrawCall::Type::Circle, x0, y0, 0, 0, (int16_t)color, 0, (int16_t)r, ""});
}

// --- GNSS ---
SpNavTime SpGnss::mockTimeData     = {2023, 10, 1, 12, 30, 0, 0};
float     SpGnss::mockVelocityData = 5.5f;

int SpGnss::mockBeginResult = 0;
int SpGnss::mockStartResult = 0;

extern unsigned long _mock_millis;

int SpGnss::begin() {
  return mockBeginResult;
}
int SpGnss::start(int mode) {
  (void)mode;
  return mockStartResult;
}
int SpGnss::stop() {
  return 0;
}
void SpGnss::select(int satelliteSystem) {
  (void)satelliteSystem;
}
bool SpGnss::waitUpdate(int timeout) {
  (void)timeout;
  if (_mock_millis > 0 && (_mock_millis % 1000 == 0)) { return true; }
  return false;
}
void SpGnss::getNavData(SpNavData *navData) {
  if (navData) {
    navData->velocity      = mockVelocityData;
    navData->time          = mockTimeData;
    navData->posFixMode    = Fix3D;
    navData->numSatellites = 8;
  }
}
