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
  OLED     oled;
  Input    input;
  Gnss     gnss;
  Mode     mode;
  Trip     trip;
  Clock    clock;
  Renderer renderer;

public:
  void begin() {
    oled.begin();
    input.begin();
    gnss.begin();
    trip.begin();
  }

  void update() {
    handleInput();

    gnss.update();
    const SpNavData &navData = gnss.getNavData();

    trip.update(navData, millis());
    clock.update(navData);

    Frame frame(trip, clock, mode.get(), (SpFixMode)navData.posFixMode);
    renderer.render(oled, frame);
  }

private:
  void handleInput() {
    switch (input.update()) {
    case Input::ID::SELECT:
      mode.next();
      return;
    case Input::ID::PAUSE:
      trip.pause();
      return;
    case Input::ID::BOTH:
      trip.reset();
      return;
    case Input::ID::NONE:
      return;
    }
  }
};
