#pragma once

#include <GNSS.h>
#include <cstring>

#include "../hardware/OLED.h"
#include "Frame.h"

class Renderer {
public:
  struct Layout {
    int16_t headerHeight;
    int16_t headerTextSize;
    int16_t headerLineYOffset;
    int16_t mainAreaYOffset;
    int16_t mainValSize;
    int16_t mainUnitSize;
    int16_t subValSize;
    int16_t subUnitSize;
    int16_t unitSpacing;
  };

  static constexpr int16_t DEFAULT_HEADER_HEIGHT        = 12;
  static constexpr int16_t DEFAULT_HEADER_TEXT_SIZE     = 1;
  static constexpr int16_t DEFAULT_HEADER_LINE_Y_OFFSET = 2;
  static constexpr int16_t DEFAULT_MAIN_AREA_Y_OFFSET   = 14;
  static constexpr int16_t DEFAULT_MAIN_VAL_SIZE        = 3;
  static constexpr int16_t DEFAULT_MAIN_UNIT_SIZE       = 1;
  static constexpr int16_t DEFAULT_SUB_VAL_SIZE         = 2;
  static constexpr int16_t DEFAULT_SUB_UNIT_SIZE        = 1;
  static constexpr int16_t DEFAULT_UNIT_SPACING         = 4;

private:
  const Layout layout;
  Frame        lastFrame;
  bool         firstRender = true;

public:
  Renderer(const Layout &layoutConfig) : layout(layoutConfig) {}

  void render(OLED &oled, Frame &frame) {
    if (!firstRender && frame == lastFrame) return;

    firstRender = false;
    lastFrame   = frame;

    oled.clear();
    drawHeader(oled, frame);
    drawMainArea(oled, frame);
    oled.display();
  }

private:
  void drawHeader(OLED &oled, const Frame &frame) {
    oled.setTextSize(layout.headerTextSize);
    oled.setTextColor(WHITE);

    drawTextLeft(oled, 0, frame.header.fixStatus);
    drawTextCenter(oled, 0, frame.header.modeSpeed);
    drawTextRight(oled, 0, frame.header.modeTime);

    int16_t lineY = layout.headerHeight - layout.headerLineYOffset;
    oled.drawLine(0, lineY, oled.getWidth(), lineY, WHITE);
  }

  void drawMainArea(OLED &oled, const Frame &frame) {
    const int16_t headerH = layout.headerHeight;
    const int16_t screenH = oled.getHeight();

    drawItem(oled, frame.main, headerH + layout.mainAreaYOffset, layout.mainValSize,
             layout.mainUnitSize, false);
    drawItem(oled, frame.sub, screenH, layout.subValSize, layout.subUnitSize, true);
  }

  void drawItem(OLED &oled, const Frame::Item &item, int16_t y, uint8_t valSize, uint8_t unitSize,
                bool alignBottom) {
    const int16_t spacing = layout.unitSpacing;

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
