#include "App.h"

App::App(OLED &displayData, Gnss &gnssData, Input &inputModule)
    : display(displayData), input(inputModule), gnss(gnssData), clock(Config::Time::JST_OFFSET, 2025) {}

void App::begin() {
  display.begin();
  input.begin();
  gnss.begin();
  trip.begin();
}

void App::update() {
  handleInput();
  gnss.update();
  const auto &navData = gnss.getNavData();
  trip.update(navData, millis());
  clock.update(navData);
  renderer.render(display, trip, clock, mode.get(), gnss.isFixed());
}

void App::handleInput() {
  InputEvent event = input.update();

  switch (event) {
  case InputEvent::BTN_A:
    mode.next();
    break;
  case InputEvent::BTN_B:
    mode.prev();
    break;
  case InputEvent::BTN_BOTH:
    trip.reset();
    break;
  default:
    break;
  }
}
