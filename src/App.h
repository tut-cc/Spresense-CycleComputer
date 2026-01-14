#pragma once

#include "domain/Clock.h"
#include "domain/Trip.h"
#include "hardware/Gnss.h"
#include "hardware/OLED.h"
#include "ui/Frame.h"
#include "ui/Input.h"
#include "ui/Mode.h"
#include "ui/Renderer.h"

class App {
private:
  OLED  &display;
  Input &input;
  Gnss  &gnss;
  Mode   mode;
  Trip   trip;
  Clock  clock;

  Renderer<OLED> renderer;

public:
  App(OLED &displayData, Gnss &gnssData, Input &inputModule)
      : display(displayData), input(inputModule), gnss(gnssData),
        clock(Config::Time::JST_OFFSET, 2025) {}

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

    Frame frame(trip, clock, mode.get(), (SpFixMode)navData.posFixMode, navData.numSatellites);
    renderer.render(display, frame);
  }

private:
  void handleInput() {
    switch (input.update()) {
    case Input::ID::BTN_A:
      mode.next();
      return;
    case Input::ID::BTN_B:
      mode.prev();
      return;
    case Input::ID::BTN_BOTH:
      trip.reset();
      return;
    case Input::ID::NONE:
      return;
    }
  }
};
