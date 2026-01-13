#pragma once

#include <Arduino.h>
#include <GNSS.h>
#include <cstring>

#include "Frame.h"

template <typename ScreenT> class Renderer {
private:
  Frame lastFrame;
  bool  firstRender = true;

  // Layout Constants
  static constexpr int16_t HEADER_HEIGHT = 12;
  static constexpr int16_t FOOTER_HEIGHT = 12;

public:
  void render(ScreenT &screen, const Frame &frame) {
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
  void drawHeader(ScreenT &screen, const char *fixStatus, const char *numSatellites) {
    screen.setTextSize(1);
    screen.setTextColor(1); // WHITE
    screen.setCursor(0, 0);
    screen.print(fixStatus);

    // Right-aligned satellite count
    int16_t  x1, y1;
    uint16_t w, h;
    screen.getTextBounds(numSatellites, 0, 0, &x1, &y1, &w, &h);
    screen.setCursor(screen.getWidth() - w, 0);
    screen.print(numSatellites);

    int16_t lineY = HEADER_HEIGHT - 2;
    screen.drawLine(0, lineY, screen.getWidth(), lineY, 1); // WHITE
  }

  void drawMainArea(ScreenT &screen, const char *value) {
    int16_t contentTop = HEADER_HEIGHT;
    int16_t contentY   = screen.getHeight() - FOOTER_HEIGHT;

    // Value (Large, Centered in remaining space)
    screen.setTextSize(4);
    int16_t  x1, y1;
    uint16_t w, h;
    screen.getTextBounds(value, 0, 0, &x1, &y1, &w, &h);
    int16_t valueY = (contentY + contentTop - h) / 2;
    screen.setCursor((screen.getWidth() - w) / 2, valueY);
    screen.print(value);
  }

  void drawFooter(ScreenT &screen, const char *footerTime, const char *footerMode, const char *unit) {
    int16_t lineY = screen.getHeight() - FOOTER_HEIGHT;
    screen.drawLine(0, lineY, screen.getWidth(), lineY, 1); // WHITE

    screen.setTextSize(1);
    screen.setTextColor(1); // WHITE
    int16_t  x1, y1;
    uint16_t w, h;
    screen.getTextBounds(footerTime, 0, 0, &x1, &y1, &w, &h);

    // Center the time in the footer vertically
    int16_t textY = lineY + (FOOTER_HEIGHT - h) / 2 + 1;

    // Right-aligned Time
    screen.setCursor(screen.getWidth() - w, textY);
    screen.print(footerTime);

    // Left-aligned Mode + Unit
    screen.setCursor(0, textY);
    screen.print(footerMode);

    if (strlen(unit) > 0) {
      screen.print(" ");
      screen.print(unit);
    }
  }
};
