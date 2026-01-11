#pragma once

#include <Arduino.h>
#include <cstring>

#include "../domain/Clock.h"
#include "../domain/Trip.h"
#include "Formatter.h"
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

  char     lastRenderedValue[32] = "";
  Mode::ID lastRenderedMode      = Mode::ID::SPEED;
  bool     firstRender           = true;

public:
  void render(ContextT &ctx, const domain::Trip &trip, const domain::Clock &clock, Mode::ID modeId) {
    char currentBuf[32];
    getDisplayValue(trip, clock, modeId, currentBuf, sizeof(currentBuf));
    if (!firstRender && modeId == lastRenderedMode && strcmp(currentBuf, lastRenderedValue) == 0) return;

    // Update Cache
    firstRender      = false;
    lastRenderedMode = modeId;
    strcpy(lastRenderedValue, currentBuf);

    ctx.clear();
    drawHeader(ctx);
    Metadata meta = getMetadata(modeId);
    drawMainArea(ctx, meta.title, currentBuf, meta.unit);
    drawFooter(ctx);
    ctx.display();
  }

private:
  void getDisplayValue(const domain::Trip &trip, const domain::Clock &clock, Mode::ID modeId, char *buf, size_t size) {
    switch (modeId) {
    case Mode::ID::SPEED:
      Formatter::formatSpeed(trip.getSpeedKmh(), buf, size);
      break;
    case Mode::ID::MAX_SPEED:
      Formatter::formatSpeed(trip.getMaxSpeedKmh(), buf, size);
      break;
    case Mode::ID::AVG_SPEED:
      Formatter::formatSpeed(trip.getAvgSpeedKmh(), buf, size);
      break;
    case Mode::ID::DISTANCE:
      Formatter::formatDistance(trip.getDistanceKm(), buf, size);
      break;
    case Mode::ID::TIME: {
      int h, m, s;
      clock.getTime(h, m, s);
      Formatter::formatTime(h, m, buf, size);
      break;
    }
    case Mode::ID::MOVING_TIME:
      Formatter::formatDuration(trip.getMovingTimeMs(), buf, size);
      break;
    case Mode::ID::ELAPSED_TIME:
      Formatter::formatDuration(trip.getElapsedTimeMs(), buf, size);
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
};

} // namespace ui
