#pragma once

#include <Arduino.h>
#include <cstring>

#include "../domain/Clock.h"
#include "../domain/Trip.h"
#include "Formatter.h"
#include "Mode.h"
#include <GNSS.h>

template <typename ContextT> class Renderer {
private:
  struct Metadata {
    const char *title;
    const char *unit;
  };

  struct Frame {
    Mode::ID  modeId;
    SpFixMode fixMode;
    int       numSatellites;
    char      value[32];

    bool operator==(const Frame &other) const {
      return modeId == other.modeId && fixMode == other.fixMode && numSatellites == other.numSatellites && strcmp(value, other.value) == 0;
    }
  };

  Frame lastFrame;
  bool  firstRender = true;

  // Layout Constants
  static constexpr int16_t HEADER_HEIGHT = 12;
  static constexpr int16_t FOOTER_HEIGHT = 12;

public:
  void render(ContextT &ctx, const Trip &trip, const Clock &clock, Mode::ID modeId, SpFixMode fixMode, int numSatellites) {
    Frame currentFrame;
    currentFrame.modeId        = modeId;
    currentFrame.fixMode       = fixMode;
    currentFrame.numSatellites = numSatellites;
    getDisplayValue(trip, clock, modeId, currentFrame.value, sizeof(currentFrame.value));

    if (!firstRender && currentFrame == lastFrame) return;

    // Update Cache
    firstRender = false;
    lastFrame   = currentFrame;

    ctx.clear();
    drawHeader(ctx, currentFrame.fixMode, currentFrame.numSatellites);
    Metadata meta = getMetadata(currentFrame.modeId);
    drawMainArea(ctx, meta.title, currentFrame.value, meta.unit);
    drawFooter(ctx);
    ctx.display();
  }

private:
  void getDisplayValue(const Trip &trip, const Clock &clock, Mode::ID modeId, char *buf, size_t size) {
    switch (modeId) {
    case Mode::ID::SPEED:
      Formatter::formatSpeed(trip.speedometer.get(), buf, size);
      return;
    case Mode::ID::MAX_SPEED:
      Formatter::formatSpeed(trip.speedometer.getMax(), buf, size);
      return;
    case Mode::ID::AVG_SPEED:
      Formatter::formatSpeed(trip.getAvgSpeedKmh(), buf, size);
      return;
    case Mode::ID::DISTANCE:
      Formatter::formatDistance(trip.odometer.getDistance(), buf, size);
      return;
    case Mode::ID::TIME: {
      Clock::Time t = clock.getTime();
      Formatter::formatTime(t.hour, t.minute, buf, size);
      return;
    }
    case Mode::ID::MOVING_TIME:
      Formatter::formatDuration(trip.stopwatch.getMovingTimeMs(), buf, size);
      return;
    case Mode::ID::ELAPSED_TIME:
      Formatter::formatDuration(trip.stopwatch.getElapsedTimeMs(), buf, size);
      return;
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

  void drawHeader(ContextT &ctx, SpFixMode fixMode, int numSatellites) {
    ctx.setTextSize(1);
    ctx.setTextColor(1); // WHITE
    ctx.setCursor(0, 0);
    switch (fixMode) {
    case FixInvalid:
      ctx.print("WAIT");
      break;
    case Fix2D:
      ctx.print("2D");
      break;
    case Fix3D:
      ctx.print("3D");
      break;
    }

    // Right-aligned satellite count
    char satBuf[8];
    snprintf(satBuf, sizeof(satBuf), "St:%d", numSatellites);
    int16_t  x1, y1;
    uint16_t w, h;
    ctx.getTextBounds(satBuf, 0, 0, &x1, &y1, &w, &h);
    ctx.setCursor(ctx.getWidth() - w, 0);
    ctx.print(satBuf);

    int16_t lineY = HEADER_HEIGHT - 2;
    ctx.drawLine(0, lineY, ctx.getWidth(), lineY, 1); // WHITE
  }

  void drawMainArea(ContextT &ctx, const char *title, const char *value, const char *unit) {
    int16_t contentTop = HEADER_HEIGHT;
    int16_t contentY   = ctx.getHeight() - FOOTER_HEIGHT;

    // Title (Top Left of Main Area)
    ctx.setTextSize(1);
    ctx.setCursor(0, contentTop + 2);
    ctx.print(title);

    // Value (Large, Centered in remaining space)
    ctx.setTextSize(2);
    int16_t  x1, y1;
    uint16_t w, h;
    ctx.getTextBounds(value, 0, 0, &x1, &y1, &w, &h);
    int16_t valueY = (contentY + contentTop - h) / 2;
    ctx.setCursor((ctx.getWidth() - w) / 2, valueY);
    ctx.print(value);

    // Unit (Bottom Right of Main Area)
    if (strlen(unit) != 0) {
      ctx.setTextSize(1);
      ctx.getTextBounds(unit, 0, 0, &x1, &y1, &w, &h);
      ctx.setCursor(ctx.getWidth() - w, contentY - h - 2);
      ctx.print(unit);
    }
  }

  void drawFooter(ContextT &ctx) {
    int16_t lineY = ctx.getHeight() - FOOTER_HEIGHT;
    ctx.drawLine(0, lineY, ctx.getWidth(), lineY, 1); // WHITE

    ctx.setTextSize(1);
    int16_t textH = 8; // Approx height for size 1
    int16_t textY = lineY + (FOOTER_HEIGHT - textH) / 2 + 1;
    ctx.setCursor(0, textY);
    ctx.print("Ready"); // Placeholder for status
  }
};
