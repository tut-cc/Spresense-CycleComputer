#pragma once
#include "Config.h"

#include "domain/Clock.h"
#include "domain/Trip.h"
#include "ui/InputEvent.h"
#include "ui/Mode.h"
#include "ui/Renderer.h"
#include <cstdio>

namespace application {

template <typename DisplayT, typename GnssT, typename InputT> class App {
private:
  DisplayT              &display;
  InputT                &input;
  GnssT                 &gnss;
  ui::Mode               mode;
  domain::Trip           trip;
  domain::Clock          clock;
  ui::Renderer<DisplayT> renderer;

public:
  App(DisplayT &displayData, GnssT &gnss, InputT &inputModule)
      : display(displayData), input(inputModule), gnss(gnss), clock(Config::Time::JST_OFFSET, 2025) {}

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
