#pragma once

#include "Config.h"
#include "system/Clock.h"
#include "system/DisplayData.h"
#include "system/InputEvent.h"
#include "system/Mode.h"
#include "system/Trip.h"
#include <cstdio>

namespace application {

template <typename DisplayT, typename GnssT, typename InputT> class CycleComputer {
private:
  DisplayT     &display;
  InputT       &input;
  GnssT        &gnss;
  Mode          mode;
  Trip          trip;
  Clock         clock;
  unsigned long lastDisplayUpdate = 0;
  bool          forceUpdate       = false;

public:
  CycleComputer(DisplayT &displayData, GnssT &gnss, InputT &input) : display(displayData), gnss(gnss), input(input) {}

  void begin() {
    display.begin();
    input.begin();
    gnss.begin();
    trip.begin();
  }

  void update() {
    handleInput();
    gnss.update();
    const auto &navData = gnss.getNavData();
    trip.update(navData, millis());
    clock.update(navData);
    updateDisplay();
  }

private:
  void handleInput() {
    InputEvent event = input.update();

    switch (event) {
    case InputEvent::BTN_A:
      mode.next();
      forceUpdate = true;
      break;
    case InputEvent::BTN_B:
      mode.prev();
      forceUpdate = true;
      break;
    case InputEvent::BTN_BOTH:
      trip.reset();
      forceUpdate = true;
      break;
    default:
      break;
    }
  }

  void updateDisplay() {
    unsigned long currentMillis = millis();

    if (!forceUpdate && (currentMillis - lastDisplayUpdate < Config::DISPLAY_UPDATE_INTERVAL_MS)) return;

    lastDisplayUpdate = currentMillis;
    forceUpdate       = false;

    char            buf[32];
    DisplayDataType type;
    getDisplayData(mode.get(), type, buf, sizeof(buf));

    display.show(type, buf);
  }

  void getDisplayData(Mode::ID modeId, DisplayDataType &type, char *buf, size_t size) {
    switch (modeId) {
    case Mode::ID::SPEED:
      type = DisplayDataType::SPEED;
      trip.getSpeedStr(buf, size);
      break;
    case Mode::ID::MAX_SPEED:
      type = DisplayDataType::MAX_SPEED;
      trip.getMaxSpeedStr(buf, size);
      break;
    case Mode::ID::AVG_SPEED:
      type = DisplayDataType::AVG_SPEED;
      trip.getAvgSpeedStr(buf, size);
      break;
    case Mode::ID::DISTANCE:
      type = DisplayDataType::DISTANCE;
      trip.getDistanceStr(buf, size);
      break;
    case Mode::ID::TIME:
      type = DisplayDataType::TIME;
      clock.getTimeStr(buf, size);
      break;
    case Mode::ID::MOVING_TIME:
      type = DisplayDataType::MOVING_TIME;
      trip.getMovingTimeStr(buf, size);
      break;
    case Mode::ID::ELAPSED_TIME:
      type = DisplayDataType::ELAPSED_TIME;
      trip.getElapsedTimeStr(buf, size);
      break;
    default:
      type   = DisplayDataType::INVALID;
      buf[0] = '\0';
      break;
    }
  }
};

} // namespace application
