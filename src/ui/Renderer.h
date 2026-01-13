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
    drawMainArea(screen, frame.title, frame.value, frame.unit);
    drawFooter(screen);
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

  void drawMainArea(ScreenT &screen, const char *title, const char *value, const char *unit) {
    int16_t contentTop = HEADER_HEIGHT;
    int16_t contentY   = screen.getHeight() - FOOTER_HEIGHT;

    // Title (Top Left of Main Area)
    screen.setTextSize(1);
    screen.setCursor(0, contentTop + 2);
    screen.print(title);

    // Value (Large, Centered in remaining space)
    screen.setTextSize(2);
    int16_t  x1, y1;
    uint16_t w, h;
    screen.getTextBounds(value, 0, 0, &x1, &y1, &w, &h);
    int16_t valueY = (contentY + contentTop - h) / 2;
    screen.setCursor((screen.getWidth() - w) / 2, valueY);
    screen.print(value);

    // Unit (Bottom Right of Main Area)
    if (strlen(unit) != 0) {
      screen.setTextSize(1);
      screen.getTextBounds(unit, 0, 0, &x1, &y1, &w, &h);
      screen.setCursor(screen.getWidth() - w, contentY - h - 2);
      screen.print(unit);
    }
  }

  void drawFooter(ScreenT &screen) {
    int16_t lineY = screen.getHeight() - FOOTER_HEIGHT;
    screen.drawLine(0, lineY, screen.getWidth(), lineY, 1); // WHITE
  }
};
