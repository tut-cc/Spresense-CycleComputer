#pragma once

#include <stddef.h>

#include "Config.h"
#include "domain/AutoSaver.h"
#include "domain/Clock.h"
#include "domain/DataStore.h"
#include "domain/Trip.h"
#include "hardware/Gnss.h"
#include "hardware/OLED.h"
#include "ui/Frame.h"
#include "ui/FrameBuilder.h"
#include "ui/Input.h"
#include "ui/Mode.h"
#include "ui/Renderer.h"

class App {
private:
  OLED      oled;
  Input     input;
  Gnss      gnss;
  Mode      mode;
  Trip      trip;
  Clock     clock;
  Renderer  renderer;
  DataStore dataStore;

  AutoSaver autoSaver;

public:
  App()
      : oled(OLED::WIDTH, OLED::HEIGHT), input(Pin::BTN_A, Pin::BTN_B),
        renderer({
            .headerHeight      = Renderer::DEFAULT_HEADER_HEIGHT,
            .headerTextSize    = Renderer::DEFAULT_HEADER_TEXT_SIZE,
            .headerLineYOffset = Renderer::DEFAULT_HEADER_LINE_Y_OFFSET,
            .mainAreaYOffset   = Renderer::DEFAULT_MAIN_AREA_Y_OFFSET,
            .mainValSize       = Renderer::DEFAULT_MAIN_VAL_SIZE,
            .mainUnitSize      = Renderer::DEFAULT_MAIN_UNIT_SIZE,
            .subValSize        = Renderer::DEFAULT_SUB_VAL_SIZE,
            .subUnitSize       = Renderer::DEFAULT_SUB_UNIT_SIZE,
            .unitSpacing       = Renderer::DEFAULT_UNIT_SPACING,
        }),
        autoSaver(dataStore, trip) {}

  void begin() {
    oled.begin(OLED::ADDRESS);
    input.begin();
    gnss.begin();
    trip.begin();

    AppData savedData = dataStore.load();

    trip.odometer.setTotalDistance(savedData.totalDistance);
    trip.setTripDistance(savedData.tripDistance);
    trip.setMovingTime(savedData.movingTimeMs);

    autoSaver.begin();
  }

  void update() {
    handleInput();

    gnss.update();
    const NavData navData = gnss.getNavData();

    trip.update(navData, millis());
    clock.update(navData);

    autoSaver.update(millis());

    Frame frame = FrameBuilder::build(trip, clock, *mode.getCurrentState(), navData.fixType);
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
    case Input::ID::RESET:
      mode.reset(trip, dataStore);
      return;
    case Input::ID::NONE:
      return;
    }
  }
};
