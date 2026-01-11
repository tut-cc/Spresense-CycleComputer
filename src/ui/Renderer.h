#pragma once

#include <Arduino.h>
#include <cstring>

#include "../domain/Clock.h"
#include "../domain/Trip.h"
#include "Mode.h"

namespace ui {

template <typename ContextT> class Renderer {
private:
  int batteryLevel   = 85;
  int satelliteCount = 5;

  struct Metadata {
    const char *title;
    const char *unit;
  };

public:
  void render(ContextT &ctx, const domain::Trip &trip, const domain::Clock &clock, Mode::ID modeId) {
    ctx.clear();

    drawHeader(ctx);

    char buf[32];
    getDisplayValue(trip, clock, modeId, buf, sizeof(buf));

    Metadata meta = getMetadata(modeId);
    drawMainArea(ctx, meta.title, buf, meta.unit);

    drawFooter(ctx);

    ctx.display();
  }

private:
  void getDisplayValue(const domain::Trip &trip, const domain::Clock &clock, Mode::ID modeId, char *buf, size_t size) {
    switch (modeId) {
    case Mode::ID::SPEED:
      trip.getSpeedStr(buf, size);
      break;
    case Mode::ID::MAX_SPEED:
      trip.getMaxSpeedStr(buf, size);
      break;
    case Mode::ID::AVG_SPEED:
      trip.getAvgSpeedStr(buf, size);
      break;
    case Mode::ID::DISTANCE:
      trip.getDistanceStr(buf, size);
      break;
    case Mode::ID::TIME:
      clock.getTimeStr(buf, size);
      break;
    case Mode::ID::MOVING_TIME:
      trip.getMovingTimeStr(buf, size);
      break;
    case Mode::ID::ELAPSED_TIME:
      trip.getElapsedTimeStr(buf, size);
      break;
    default:
      buf[0] = '\0';
      break;
    }
  }

  Metadata getMetadata(Mode::ID modeId) {
    switch (modeId) {
    case Mode::ID::SPEED:
      return {"SPEED", "km/h"};
    case Mode::ID::MAX_SPEED:
      return {"MAX SPEED", "km/h"};
    case Mode::ID::AVG_SPEED:
      return {"AVG SPEED", "km/h"};
    case Mode::ID::TIME:
      return {"TIME", ""};
    case Mode::ID::MOVING_TIME:
      return {"TRIP TIME", ""};
    case Mode::ID::ELAPSED_TIME:
      return {"ELAPSED TIME", ""};
    case Mode::ID::DISTANCE:
      return {"DISTANCE", "km"};
    default:
      return {"", ""};
    }
  }

  // Taken from OLED.h
  void drawHeader(ContextT &ctx) {
    ctx.setTextSize(1);
    ctx.setTextColor(1); // WHITE
    ctx.setCursor(0, 0);
    ctx.print("GNSS ON");

    drawSatelliteIcon(ctx, 100, 0, satelliteCount);
    drawBatteryIcon(ctx, 115, 0, batteryLevel);

    ctx.drawLine(0, 10, ctx.getWidth(), 10, 1); // WHITE
  }

  void drawFooter(ContextT &ctx) {
    ctx.drawLine(0, ctx.getHeight() - 10, ctx.getWidth(), ctx.getHeight() - 10, 1); // WHITE

    ctx.setTextSize(1);
    ctx.setCursor(0, ctx.getHeight() - 8);
    ctx.print("Ready"); // Placeholder for status
  }

  void drawMainArea(ContextT &ctx, const char *title, const char *value, const char *unit) {
    // Title
    ctx.setTextSize(1);
    ctx.setCursor(0, 14);
    ctx.print(title);

    // Value (Large)
    ctx.setTextSize(2); // Make value bigger
    int16_t  x1, y1;
    uint16_t w, h;
    ctx.getTextBounds(value, 0, 0, &x1, &y1, &w, &h);
    ctx.setCursor((ctx.getWidth() - w) / 2, 28);
    ctx.print(value);

    if (strlen(unit) == 0) return;

    // Unit
    ctx.setTextSize(1);
    ctx.setCursor(ctx.getWidth() - 24, 45); // Bottom right of main area
    ctx.print(unit);
  }

  void drawBatteryIcon(ContextT &ctx, int x, int y, int percentage) {
    ctx.drawRect(x, y, 12, 6, 1);
    ctx.fillRect(x + 12, y + 2, 2, 2, 1); // Battery positive terminal

    int width = map(percentage, 0, 100, 0, 10);
    ctx.fillRect(x + 1, y + 1, width, 4, 1);
  }

  void drawSatelliteIcon(ContextT &ctx, int x, int y, int count) {
    ctx.drawCircle(x + 3, y + 3, 2, 1); // Placeholder for satellite icon
  }
};

} // namespace ui
