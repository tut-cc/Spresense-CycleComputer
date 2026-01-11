#pragma once

#include "Config.h"
#include "domain/Clock.h"
#include "domain/Trip.h"
#include "ui/Input.h"
#include "ui/InputEvent.h"
#include "ui/Mode.h"
#include "ui/Renderer.h"
#include <cstdio>

namespace application {

template <typename DisplayT, typename GnssT, typename ButtonT> class CycleComputer {
private:
  DisplayT              &display;
  ui::Input<ButtonT>     input;
  GnssT                 &gnss;
  ui::Mode               mode;
  domain::Trip           trip;
  domain::Clock          clock;
  ui::Renderer<DisplayT> renderer;

public:
  CycleComputer(DisplayT &displayData, GnssT &gnss, ButtonT &btnA, ButtonT &btnB) : display(displayData), input(btnA, btnB), gnss(gnss) {}

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
    renderer.render(display, trip, clock, mode.get());
  }

private:
  void handleInput() {
    ui::InputEvent event = input.update();

    switch (event) {
    case ui::InputEvent::BTN_A:
      mode.next();
      break;
    case ui::InputEvent::BTN_B:
      mode.prev();
      break;
    case ui::InputEvent::BTN_BOTH:
      trip.reset();
      break;
    default:
      break;
    }
  }
};

} // namespace application
