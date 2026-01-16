#pragma once

#include <GNSS.h>
#include <cstdio>
#include <cstring>

#include "../domain/Clock.h"
#include "../hardware/OLED.h"

// ==========================================
// Frame
// ==========================================
struct Frame {
  struct Item {
    char value[16] = "";
    char unit[16]  = "";

    bool operator==(const Item &other) const {
      return strcmp(value, other.value) == 0 && strcmp(unit, other.unit) == 0;
    }
  };

  struct Header {
    char fixStatus[8] = "";
    char modeSpeed[8] = "";
    char modeTime[8]  = "";

    bool operator==(const Header &other) const {
      const bool fixStatusEq = strcmp(fixStatus, other.fixStatus) == 0;
      const bool modeSpeedEq = strcmp(modeSpeed, other.modeSpeed) == 0;
      const bool modeTimeEq  = strcmp(modeTime, other.modeTime) == 0;
      return fixStatusEq && modeSpeedEq && modeTimeEq;
    }
  };

  Header header;
  Item   main;
  Item   sub;

  Frame() = default;

  bool operator==(const Frame &other) const {
    return header == other.header && main == other.main && sub == other.sub;
  }
};

// ==========================================
// Formatter
// ==========================================
namespace Formatter {

inline void formatSpeed(float speedKmh, char *buffer, size_t size) {
  snprintf(buffer, size, "%4.1f", speedKmh);
}

inline void formatDistance(float distanceKm, char *buffer, size_t size) {
  snprintf(buffer, size, "%5.2f", distanceKm);
}

inline void formatTime(const Clock::Time time, char *buffer, size_t size) {
  snprintf(buffer, size, "%02d:%02d", time.hour, time.minute);
}

inline void formatDuration(unsigned long millis, char *buffer, size_t size) {
  const unsigned long seconds = millis / 1000;
  const unsigned long h       = seconds / 3600;
  const unsigned long m       = (seconds % 3600) / 60;
  const unsigned long s       = seconds % 60;

  if (0 < h) snprintf(buffer, size, "%lu:%02lu:%02lu", h, m, s);
  else snprintf(buffer, size, "%02lu:%02lu", m, s);
}

} // namespace Formatter

class Renderer {
  static constexpr int16_t HEADER_HEIGHT        = 12;
  static constexpr int16_t HEADER_TEXT_SIZE     = 1;
  static constexpr int16_t HEADER_LINE_Y_OFFSET = 2;
  static constexpr int16_t MAIN_AREA_Y_OFFSET   = 14;
  static constexpr int16_t MAIN_VAL_SIZE        = 3;
  static constexpr int16_t MAIN_UNIT_SIZE       = 1;
  static constexpr int16_t SUB_VAL_SIZE         = 2;
  static constexpr int16_t SUB_UNIT_SIZE        = 1;
  static constexpr int16_t UNIT_SPACING         = 4;

private:
  Frame lastFrame;
  bool  firstRender = true;

public:
  Renderer() {}

  void render(OLED &oled, Frame &frame) {
    if (!firstRender && frame == lastFrame) return;

    firstRender = false;
    lastFrame   = frame;

    oled.clear();
    drawHeader(oled, frame);
    drawMainArea(oled, frame);
    oled.display();
  }

  void reset() {
    firstRender = true;
  }

private:
  void drawHeader(OLED &oled, const Frame &frame) {
    oled.setTextSize(HEADER_TEXT_SIZE);
    oled.setTextColor(WHITE);

    drawTextLeft(oled, 0, frame.header.fixStatus);
    drawTextCenter(oled, 0, frame.header.modeSpeed);
    drawTextRight(oled, 0, frame.header.modeTime);

    int16_t lineY = HEADER_HEIGHT - HEADER_LINE_Y_OFFSET;
    oled.drawLine(0, lineY, oled.getWidth(), lineY, WHITE);
  }

  void drawMainArea(OLED &oled, const Frame &frame) {
    const int16_t headerH = HEADER_HEIGHT;
    const int16_t screenH = oled.getHeight();

    drawItem(oled, frame.main, headerH + MAIN_AREA_Y_OFFSET, MAIN_VAL_SIZE, MAIN_UNIT_SIZE, false);
    drawItem(oled, frame.sub, screenH, SUB_VAL_SIZE, SUB_UNIT_SIZE, true);
  }

  void drawItem(OLED &oled, const Frame::Item &item, int16_t y, uint8_t valSize, uint8_t unitSize,
                bool alignBottom) {
    const int16_t spacing = UNIT_SPACING;

    oled.setTextSize(valSize);
    OLED::Rect valRect = oled.getTextBounds(item.value);
    oled.setTextSize(unitSize);
    OLED::Rect unitRect = oled.getTextBounds(item.unit);

    int16_t totalW = valRect.w;
    if (0 < strlen(item.unit)) totalW += spacing + unitRect.w;

    int16_t startX = (oled.getWidth() - totalW) / 2;

    int16_t valY;
    int16_t unitY;

    if (alignBottom) {
      valY  = y - valRect.h;
      unitY = y - unitRect.h;
    } else {
      valY  = y - valRect.h / 2;
      unitY = (y + valRect.h / 2) - unitRect.h;
    }

    oled.setTextSize(valSize);
    oled.setCursor(startX, valY);
    oled.print(item.value);

    if (0 < strlen(item.unit)) {
      oled.setTextSize(unitSize);
      oled.setCursor(startX + valRect.w + spacing, unitY);
      oled.print(item.unit);
    }
  }

  void drawTextLeft(OLED &oled, int16_t y, const char *text) {
    oled.setCursor(0, y);
    oled.print(text);
  }

  void drawTextCenter(OLED &oled, int16_t y, const char *text) {
    OLED::Rect rect = oled.getTextBounds(text);
    oled.setCursor((oled.getWidth() - rect.w) / 2, y);
    oled.print(text);
  }

  void drawTextRight(OLED &oled, int16_t y, const char *text) {
    OLED::Rect rect = oled.getTextBounds(text);
    oled.setCursor(oled.getWidth() - rect.w, y);
    oled.print(text);
  }
};
