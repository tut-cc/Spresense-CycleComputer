#pragma once

#include "Config.h"
#include "domain/Clock.h"
#include "domain/Trip.h"
#include "ui/DisplayData.h"
#include "ui/InputEvent.h"
#include "ui/Mode.h"
#include <cstdio>

namespace application {

template <typename DisplayT, typename GnssT, typename InputT> class CycleComputer {
private:
  DisplayT     &display;
  InputT       &input;
  GnssT        &gnss;
  ui::Mode      mode;
  domain::Trip  trip;
  domain::Clock clock;
  unsigned long lastDisplayUpdate = 0;
  bool          forceUpdate       = false;

public:
  CycleComputer(DisplayT &displayData, GnssT &gnss, InputT &input) : display(displayData), input(input), gnss(gnss) {}

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
    ui::InputEvent event = input.update();

    switch (event) {
    case ui::InputEvent::BTN_A:
      mode.next();
      forceUpdate = true;
      break;
    case ui::InputEvent::BTN_B:
      mode.prev();
      forceUpdate = true;
      break;
    case ui::InputEvent::BTN_BOTH:
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

    char                buf[32];
    ui::DisplayDataType type;
    getDisplayData(mode.get(), type, buf, sizeof(buf));

    display.show(type, buf);
  }

  void getDisplayData(ui::Mode::ID modeId, ui::DisplayDataType &type, char *buf, size_t size) {
    switch (modeId) {
    case ui::Mode::ID::SPEED:
      type = ui::DisplayDataType::SPEED;
      trip.getSpeedStr(buf, size);
      break;
    case ui::Mode::ID::MAX_SPEED:
      type = ui::DisplayDataType::MAX_SPEED;
      trip.getMaxSpeedStr(buf, size);
      break;
    case ui::Mode::ID::AVG_SPEED:
      type = ui::DisplayDataType::AVG_SPEED;
      trip.getAvgSpeedStr(buf, size);
      break;
    case ui::Mode::ID::DISTANCE:
      type = ui::DisplayDataType::DISTANCE;
      trip.getDistanceStr(buf, size);
      break;
    case ui::Mode::ID::TIME:
      type = ui::DisplayDataType::TIME;
      clock.getTimeStr(buf, size);
      break;
    case ui::Mode::ID::MOVING_TIME:
      type = ui::DisplayDataType::MOVING_TIME;
      trip.getMovingTimeStr(buf, size);
      break;
    case ui::Mode::ID::ELAPSED_TIME:
      type = ui::DisplayDataType::ELAPSED_TIME;
      trip.getElapsedTimeStr(buf, size);
      break;
    default:
      type   = ui::DisplayDataType::INVALID;
      buf[0] = '\0';
      break;
    }
  }
};

} // namespace application
