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
    drawTextLeft(oled, 0, frame.fixStatus);

    drawTextCenter(oled, 0, frame.modeLabel);

    drawTextRight(oled, 0, frame.modeTime);

    int16_t lineY = Config::Renderer::HEADER_HEIGHT - 2;
    oled.drawLine(0, lineY, oled.getWidth(), lineY, WHITE);
  }

  void drawMainArea(OLED &oled, const Frame &frame) {
    const int16_t headerH = Config::Renderer::HEADER_HEIGHT;
    const int16_t screenH = oled.getHeight();
    const int16_t screenW = oled.getWidth();

    // Draw Main Value (Size 3) and Unit (Size 1)
    {
      const int16_t valSize  = 3;
      const int16_t unitSize = 1;
      const int16_t spacing  = 4;
      const int16_t baseY    = headerH + 14; // Approximate Y for main value

      oled.setTextSize(valSize);
      OLED::Rect valRect = oled.getTextBounds(frame.mainValue);
      oled.setTextSize(unitSize);
      OLED::Rect unitRect = oled.getTextBounds(frame.mainUnit);

      int16_t totalW = valRect.w;
      if (strlen(frame.mainUnit) > 0) totalW += spacing + unitRect.w;

      int16_t startX = (screenW - totalW) / 2;

      oled.setTextSize(valSize);
      oled.setCursor(startX, baseY - valRect.h / 2); // Vertically center roughly
      oled.print(frame.mainValue);

      if (strlen(frame.mainUnit) > 0) {
        oled.setTextSize(unitSize);
        oled.setCursor(startX + valRect.w + spacing, baseY + valRect.h / 2 - unitRect.h);
        oled.print(frame.mainUnit);
      }
    }

    // Draw Sub Value (Size 2) and Unit (Size 1)
    {
      const int16_t valSize  = 2;
      const int16_t unitSize = 1;
      const int16_t spacing  = 4;
      const int16_t baseY    = screenH; // Bottom aligned

      oled.setTextSize(valSize);
      OLED::Rect valRect = oled.getTextBounds(frame.subValue);
      oled.setTextSize(unitSize);
      OLED::Rect unitRect = oled.getTextBounds(frame.subUnit);

      int16_t totalW = valRect.w;
      if (strlen(frame.subUnit) > 0) totalW += spacing + unitRect.w;

      int16_t startX = (screenW - totalW) / 2;

      oled.setTextSize(valSize);
      oled.setCursor(startX, baseY - valRect.h);
      oled.print(frame.subValue);

      if (strlen(frame.subUnit) > 0) {
        oled.setTextSize(unitSize);
        oled.setCursor(startX + valRect.w + spacing, baseY - unitRect.h);
        oled.print(frame.subUnit);
      }
    }
  }

  void drawTextLeft(OLED &oled, int16_t y, const char *text) {
    oled.setCursor(0, y);
    oled.print(text);
  }

  void drawTextRight(OLED &oled, int16_t y, const char *text) {
    OLED::Rect rect = oled.getTextBounds(text);
    oled.setCursor(oled.getWidth() - rect.w, y);
    oled.print(text);
  }

  void drawTextCenter(OLED &oled, int16_t y, const char *text) {
    OLED::Rect rect = oled.getTextBounds(text);
    // oled.setCursor((oled.getWidth() - rect.w) / 2, y - rect.h / 2);
    oled.setCursor((oled.getWidth() - rect.w) / 2, y );
    oled.print(text);
  }
};
