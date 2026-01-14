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

    // Update Cache
    firstRender = false;
    lastFrame   = frame;

    oled.clear();
    drawHeader(oled, frame.fixStatus, frame.satelliteCount);
    drawMainArea(oled, frame.value);
    drawFooter(oled, frame.footerTime, frame.footerMode, frame.unit);
    oled.display();
  }

private:
  void drawHeader(OLED &oled, char *fixStatus, char *numSatellites) {
    oled.setTextSize(1);
    oled.setTextColor(1); // WHITE
    oled.setCursor(0, 0);
    oled.print(fixStatus);

    int16_t  x1, y1;
    uint16_t w, h;
    oled.getTextBounds(numSatellites, 0, 0, &x1, &y1, &w, &h);
    oled.setCursor(oled.getWidth() - w, 0);
    oled.print(numSatellites);

    int16_t lineY = Config::Renderer::HEADER_HEIGHT - 2;
    oled.drawLine(0, lineY, oled.getWidth(), lineY, 1); // WHITE
  }

  void drawMainArea(OLED &oled, char *value) {
    int16_t contentTop = Config::Renderer::HEADER_HEIGHT;
    int16_t contentY   = oled.getHeight() - Config::Renderer::FOOTER_HEIGHT;

    oled.setTextSize(4);
    int16_t  x1, y1;
    uint16_t w, h;
    oled.getTextBounds(value, 0, 0, &x1, &y1, &w, &h);
    int16_t valueY = (contentY + contentTop - h) / 2;
    oled.setCursor((oled.getWidth() - w) / 2, valueY);
    oled.print(value);
  }

  void drawFooter(OLED &oled, char *footerTime, char *footerMode, char *unit) {
    int16_t lineY = oled.getHeight() - Config::Renderer::FOOTER_HEIGHT;
    oled.drawLine(0, lineY, oled.getWidth(), lineY, 1); // WHITE

    oled.setTextSize(1);
    oled.setTextColor(1); // WHITE
    int16_t  x1, y1;
    uint16_t w, h;
    oled.getTextBounds(footerTime, 0, 0, &x1, &y1, &w, &h);

    int16_t textY = lineY + (Config::Renderer::FOOTER_HEIGHT - h) / 2 + 1;

    oled.setCursor(oled.getWidth() - w, textY);
    oled.print(footerTime);

    oled.setCursor(0, textY);
    oled.print(footerMode);

    if (strlen(unit) > 0) {
      oled.print(" ");
      oled.print(unit);
    }
  }
};
