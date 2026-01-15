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
    drawFooter(oled, frame);
    oled.display();
  }

private:
  void drawHeader(OLED &oled, const Frame &frame) {
    oled.setTextSize(Config::Renderer::HEADER_TEXT_SIZE);
    oled.setTextColor(WHITE);
    drawTextLeft(oled, 0, frame.fixStatus);

    int16_t lineY = Config::Renderer::HEADER_HEIGHT - 2;
    oled.drawLine(0, lineY, oled.getWidth(), lineY, WHITE);
  }

  void drawMainArea(OLED &oled, const Frame &frame) {
    int16_t contentTop = Config::Renderer::HEADER_HEIGHT;
    int16_t contentY   = oled.getHeight() - Config::Renderer::FOOTER_HEIGHT;
    int16_t centerY    = (contentY + contentTop) / 2;

    oled.setTextSize(Config::Renderer::MAIN_TEXT_SIZE);
    oled.setTextColor(WHITE);
    drawTextCenter(oled, centerY, frame.value);
  }

  void drawFooter(OLED &oled, const Frame &frame) {
    int16_t lineY = oled.getHeight() - Config::Renderer::FOOTER_HEIGHT;
    oled.drawLine(0, lineY, oled.getWidth(), lineY, WHITE);

    int16_t textY = lineY + (Config::Renderer::FOOTER_HEIGHT / 2) - 3;

    oled.setTextSize(Config::Renderer::FOOTER_TEXT_SIZE);
    oled.setTextColor(WHITE);
    drawTextRight(oled, textY, frame.footerTime);

    // Draw mode and optionally unit
    drawTextLeft(oled, textY, frame.footerMode);
    if (0 < strlen(frame.unit)) {
      oled.print(" ");
      oled.print(frame.unit);
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
    oled.setCursor((oled.getWidth() - rect.w) / 2, y - rect.h / 2);
    oled.print(text);
  }
};
