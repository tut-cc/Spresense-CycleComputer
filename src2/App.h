#pragma once

#include "Config.h"
#include "domain/BatteryMonitor.h"
#include "domain/DataStore.h"
#include "domain/TripData.h"
#include "ui/DisplayFrame.h"
#include "ui/Input.h"
#include "ui/Renderer.h"
#include <GNSS.h>
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
  SpGnss                     gnss;
  DataStore                  store;
  BatteryMonitor             volt;
  Input                      input;
  Renderer                   renderer;
  Mode                       mode = Mode::SPD_TIM;
  DoubleBuffer<TripData>     trip;
  DoubleBuffer<DisplayFrame> frame;
  DoubleBuffer<SaveData>     save;
  unsigned long              now = 0, lastUi = 0, lastSave = 0, frames = 0, lastFps = 0;
  GnssData                   curGnss = {};
  Input::Event               curBtn  = Input::Event::NONE;

public:
  App() : input(Config::Pins::BUTTON_SELECT, Config::Pins::BUTTON_PAUSE) {}

  void begin() {
    Serial.begin(115200);
    bool gOk = (gnss.begin() == 0);
    if (gOk) {
      gnss.select(GPS);
      gnss.select(GLONASS);
      gnss.select(QZ_L1CA);
      gOk = (gnss.start(COLD_START) == 0);
    }

    if (!renderer.begin() || !gOk) {
      LowPower.begin();
      LowPower.deepSleep(0);
    }

    input.begin();
    volt.begin();
    SaveData s = store.load();
    trip.initialize(TripData(s));
    trip.current().clock.begin();
    save.initialize(s);
  }

  void update() {
    now             = millis();
    curBtn          = input.update();
    curGnss.updated = (gnss.waitUpdate(0) == 1);
    if (curGnss.updated) gnss.getNavData(&curGnss.navData);

    if (curBtn != Input::Event::NONE) handleButton();
    trip.apply(TripData(trip.current(), curGnss, now));

    if (true) { // Force update for FPS measurement
      if (frame.apply(DisplayFrame(trip.current(), curGnss, trip.current().clock.now(), mode))) {
        renderer.render(frame.current());
        lastUi = now;
        frames++;
      }
    }

    if (now - lastFps >= 1000) {
      Serial.print("FPS: ");
      Serial.println(frames);
      frames  = 0;
      lastFps = now;
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
      return;

    case Input::Event::PAUSE:
      s.status = s.isPaused() ? TripData::Status::Stopped : TripData::Status::Paused;
      return;

    case Input::Event::RESET:
      if (mode == Mode::SPD_TIM) s.clearAvgOdo();
      else if (mode == Mode::AVG_ODO) s.clearAllData();
      else s.clearMaxSpeed();
      return;

    case Input::Event::RESET_LONG:
      s.clearAllData();
      store.clear();
      renderer.resetDisplay();
      frame.initialize({});
      save.initialize(SaveData(s, 0));
      return;

    default:
      return;
    }
  }
};
