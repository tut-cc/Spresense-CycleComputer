#pragma once

#include <GNSS.h>
#include <cstring>

#include "../hardware/OLED.h"
#include "Frame.h"

class Renderer {
private:
  Frame lastFrame;
  bool  firstRender = true;

  // Layout Constants
  static constexpr int16_t HEADER_HEIGHT = 12;
  static constexpr int16_t FOOTER_HEIGHT = 12;

public:
  void render(OLED &screen, Frame &frame) {
    if (!firstRender && frame == lastFrame) return;

    // Update Cache
    firstRender = false;
    lastFrame   = frame;

    screen.clear();
    drawHeader(screen, frame.fixStatus, frame.satelliteCount);
    drawMainArea(screen, frame.value);
    drawFooter(screen, frame.footerTime, frame.footerMode, frame.unit);
    screen.display();
  }

private:
  void drawHeader(OLED &screen, char *fixStatus, char *numSatellites) {
    screen.setTextSize(1);
    screen.setTextColor(1); // WHITE
    screen.setCursor(0, 0);
    screen.print(fixStatus);

    int16_t  x1, y1;
    uint16_t w, h;
    screen.getTextBounds(numSatellites, 0, 0, &x1, &y1, &w, &h);
    screen.setCursor(screen.getWidth() - w, 0);
    screen.print(numSatellites);

    int16_t lineY = HEADER_HEIGHT - 2;
    screen.drawLine(0, lineY, screen.getWidth(), lineY, 1); // WHITE
  }

  void drawMainArea(OLED &screen, char *value) {
    int16_t contentTop = HEADER_HEIGHT;
    int16_t contentY   = screen.getHeight() - FOOTER_HEIGHT;

    screen.setTextSize(4);
    int16_t  x1, y1;
    uint16_t w, h;
    screen.getTextBounds(value, 0, 0, &x1, &y1, &w, &h);
    int16_t valueY = (contentY + contentTop - h) / 2;
    screen.setCursor((screen.getWidth() - w) / 2, valueY);
    screen.print(value);
  }

  void drawFooter(OLED &screen, char *footerTime, char *footerMode, char *unit) {
    int16_t lineY = screen.getHeight() - FOOTER_HEIGHT;
    screen.drawLine(0, lineY, screen.getWidth(), lineY, 1); // WHITE

    screen.setTextSize(1);
    screen.setTextColor(1); // WHITE
    int16_t  x1, y1;
    uint16_t w, h;
    screen.getTextBounds(footerTime, 0, 0, &x1, &y1, &w, &h);

    int16_t textY = lineY + (FOOTER_HEIGHT - h) / 2 + 1;

    screen.setCursor(screen.getWidth() - w, textY);
    screen.print(footerTime);

    screen.setCursor(0, textY);
    screen.print(footerMode);

    if (strlen(unit) > 0) {
      screen.print(" ");
      screen.print(unit);
    }
  }
};
