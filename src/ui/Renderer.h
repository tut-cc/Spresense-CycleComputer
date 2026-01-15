#pragma once

#include <GNSS.h>
#include <cstring>

#include "../hardware/OLED.h"
#include "Frame.h"

class Renderer {
private:
  Frame lastFrame;
  bool  firstRender = true;

public:
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
    oled.setTextSize(Config::Renderer::HEADER_TEXT_SIZE);
    oled.setTextColor(WHITE);

    drawTextLeft(oled, 0, frame.header.fixStatus);
    drawTextCenter(oled, 0, frame.header.modeSpeed);
    drawTextRight(oled, 0, frame.header.modeTime);

    int16_t lineY = Config::Renderer::HEADER_HEIGHT - 2;
    oled.drawLine(0, lineY, oled.getWidth(), lineY, WHITE);
  }

  void drawMainArea(OLED &oled, const Frame &frame) {
    const int16_t headerH = Config::Renderer::HEADER_HEIGHT;
    const int16_t screenH = oled.getHeight();

    drawItem(oled, frame.main, headerH + 14, 3, 1, false);
    drawItem(oled, frame.sub, screenH, 2, 1, true);
  }

  void drawItem(OLED &oled, const Frame::Item &item, int16_t y, uint8_t valSize, uint8_t unitSize,
                bool alignBottom) {
    const int16_t spacing = 4;

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
