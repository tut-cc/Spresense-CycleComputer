#pragma once

#include "Config.h"
#include "domain/DataStore.h"
#include "domain/TripState.h"
#include "domain/VoltageMonitor.h"
#include "hardware/Clock.h"
#include "hardware/Gnss.h"
#include "ui/DisplayFrame.h"
#include "ui/Input.h"
#include "ui/Renderer.h"
#include <LowPower.h>

template <typename T> struct DoubleBuffer {
  T    b[2];
  int  i = 0;
  T   &current() { return b[i]; }
  void initialize(const T &v) { b[0] = b[1] = v; }
  bool apply(const T &n) {
    i    = 1 - i;
    b[i] = n;
    return b[i] != b[1 - i];
  }
};

class App {
private:
  Gnss                       gnss;
  Clock                      clock;
  DataStore                  store;
  VoltageMonitor             volt;
  Input                      input;
  Renderer                   renderer;
  Mode                       mode = Mode::SPD_TIM;
  DoubleBuffer<TripState>    trip;
  DoubleBuffer<DisplayFrame> frame;
  DoubleBuffer<SaveData>     save;
  unsigned long              now = 0, lastUi = 0, lastSave = 0;
  GnssData                   curGnss = {};
  Input::Event               curBtn  = Input::Event::NONE;

public:
  App() : input(Config::Pins::BUTTON_SELECT, Config::Pins::BUTTON_PAUSE) {}

  void begin() {
    if (!renderer.begin() || !gnss.begin()) {
      LowPower.begin();
      LowPower.deepSleep(0);
    }
    input.begin();
    clock.begin();
    volt.begin();
    SaveData s = store.load();
    trip.initialize(TripState(s));
    save.initialize(s);
  }

  void update() {
    now    = millis();
    curBtn = input.update();
    if (gnss.update()) {
      curGnss.updated = true;
      curGnss.navData = gnss.navData;
      if (curGnss.navData.posFixMode >= 2) clock.sync(curGnss.navData.time);
    } else curGnss.updated = false;

    if (curBtn != Input::Event::NONE) handleButton();
    trip.apply(TripState(trip.current(), curGnss, now));

    if (curBtn != Input::Event::NONE || now - lastUi >= Config::UI::UPDATE_INTERVAL_MS) {
      if (frame.apply(DisplayFrame(trip.current(), curGnss, clock.now(), mode))) {
        renderer.render(frame.current());
        lastUi = now;
      }
    }

    if (now - lastSave >= DataStore::SAVE_INTERVAL_MS && !curGnss.updated) {
      trip.current().updateAverageSpeed();
      if (save.apply(SaveData(trip.current(), volt.update()))) store.save(save.current());
      lastSave = now;
    }
  }

private:
  void handleButton() {
    auto &s = trip.current();
    switch (curBtn) {
    case Input::Event::SELECT:
      mode = (Mode)(((int)mode + 1) % 3);
      break;
    case Input::Event::PAUSE:
      s.status = s.isPaused() ? TripState::Status::Stopped : TripState::Status::Paused;
      break;
    case Input::Event::RESET:
      if (mode == Mode::SPD_TIM) s.clearTripData();
      else if (mode == Mode::AVG_ODO) s.clearAllData();
      else s.resetMaxSpeed();
      break;
    case Input::Event::RESET_LONG:
      s.clearAllData();
      store.clear();
      renderer.resetDisplay();
      frame.initialize({});
      save.initialize(SaveData(s, 0));
      break;
    default:
      break;
    }
  }
};
