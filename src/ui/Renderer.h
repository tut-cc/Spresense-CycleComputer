#pragma once

#include <Arduino.h>
#include <cstring>

#include "../domain/Clock.h"
#include "../domain/Trip.h"
#include "Formatter.h"
#include "Mode.h"

template <typename ContextT> class Renderer {
private:
  struct Metadata {
    const char *title;
    const char *unit;
  };

  char     lastRenderedValue[32] = "";
  Mode::ID lastRenderedMode      = Mode::ID::SPEED;
  bool     firstRender           = true;

public:
  void render(ContextT &ctx, const Trip &trip, const Clock &clock, Mode::ID modeId) {
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
  void getDisplayValue(const Trip &trip, const Clock &clock, Mode::ID modeId, char *buf, size_t size) {
    switch (modeId) {
    case Mode::ID::SPEED:
      Formatter::formatSpeed(trip.speedometer.get(), buf, size);
      break;
    case Mode::ID::MAX_SPEED:
      Formatter::formatSpeed(trip.speedometer.getMax(), buf, size);
      break;
    case Mode::ID::AVG_SPEED:
      Formatter::formatSpeed(trip.getAvgSpeedKmh(), buf, size);
      break;
    case Mode::ID::DISTANCE:
      Formatter::formatDistance(trip.odometer.getDistance(), buf, size);
      break;
    case Mode::ID::TIME: {
      Clock::Time t = clock.getTime();
      Formatter::formatTime(t.hour, t.minute, buf, size);
      break;
    }
    case Mode::ID::MOVING_TIME:
      Formatter::formatDuration(trip.stopwatch.getMovingTimeMs(), buf, size);
      break;
    case Mode::ID::ELAPSED_TIME:
      Formatter::formatDuration(trip.stopwatch.getElapsedTimeMs(), buf, size);
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

  // Layout Constants
  static constexpr int16_t HEADER_HEIGHT = 12;
  static constexpr int16_t FOOTER_HEIGHT = 12;

  // Taken from OLED.h
  void drawHeader(ContextT &ctx) {
    ctx.setTextSize(1);
    ctx.setTextColor(1); // WHITE
    ctx.setCursor(0, 0);
    ctx.print("GNSS ON");

    int16_t lineY = HEADER_HEIGHT - 2;
    ctx.drawLine(0, lineY, ctx.getWidth(), lineY, 1); // WHITE
  }

  void drawFooter(ContextT &ctx) {
    int16_t lineY = ctx.getHeight() - FOOTER_HEIGHT;
    ctx.drawLine(0, lineY, ctx.getWidth(), lineY, 1); // WHITE

    ctx.setTextSize(1);
    // Align text vertically in footer
    int16_t textH = 8; // Approx height for size 1
    int16_t textY = lineY + (FOOTER_HEIGHT - textH) / 2 + 1;
    ctx.setCursor(0, textY);
    ctx.print("Ready"); // Placeholder for status
  }

  void drawMainArea(ContextT &ctx, const char *title, const char *value, const char *unit) {
    int16_t contentTop = HEADER_HEIGHT;
    int16_t contentY   = ctx.getHeight() - FOOTER_HEIGHT;

    // Title (Top Left of Content Area)
    ctx.setTextSize(1);
    ctx.setCursor(0, contentTop + 2);
    ctx.print(title);

    // Value (Large, Centered in remaining space)
    ctx.setTextSize(2); // Make value bigger
    int16_t  x1, y1;
    uint16_t w, h;
    ctx.getTextBounds(value, 0, 0, &x1, &y1, &w, &h);

    // Vertically center value in the content area
    // (contentY + contentTop) / 2 yields the visual center y
    // Subtract h/2 to place the top-left corner of the text
    int16_t valueY = (contentY + contentTop - h) / 2;

    ctx.setCursor((ctx.getWidth() - w) / 2, valueY);
    ctx.print(value);

    if (strlen(unit) == 0) return;

    // Unit (Bottom Right of Content Area)
    ctx.setTextSize(1);
    ctx.getTextBounds(unit, 0, 0, &x1, &y1, &w, &h);
    ctx.setCursor(ctx.getWidth() - w, contentY - h - 2);
    ctx.print(unit);
  }
};
