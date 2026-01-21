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
  T    buffers[2];
  int  index = 0;
  T   &current() { return buffers[index]; }
  void initialize(const T &value) { buffers[0] = buffers[1] = value; }
  bool apply(const T &newValue) {
    index          = 1 - index;
    buffers[index] = newValue;
    return buffers[index] != buffers[1 - index];
  }
};

class App {
private:
  SpGnss                     gnss;
  DataStore                  store;
  BatteryMonitor             batteryMonitor;
  Input                      input;
  Renderer                   renderer;
  Mode                       mode = Mode::SPD_TIM;
  DoubleBuffer<TripData>     trip;
  DoubleBuffer<DisplayFrame> frame;
  DoubleBuffer<SaveData>     save;
  unsigned long              now = 0, lastUi = 0, lastSave = 0, loops = 0, lastFps = 0;
  GnssData                   curGnss = {};
  Input::Event               curBtn  = Input::Event::NONE;

public:
  App() : input(Config::Pins::BUTTON_SELECT, Config::Pins::BUTTON_PAUSE) {}

  void begin() {
    Serial.begin(115200);
    bool gnssInitialized = (gnss.begin() == 0);
    if (gnssInitialized) {
      gnss.select(GPS);
      gnss.select(GLONASS);
      gnss.select(QZ_L1CA);
      gnssInitialized = (gnss.start(COLD_START) == 0);
    }

    if (!renderer.begin() || !gnssInitialized) {
      LowPower.begin();
      LowPower.deepSleep(0);
    }

    input.begin();
    batteryMonitor.begin();
    SaveData savedData = store.load();
    trip.initialize(savedData.toTripData());
    trip.current().clock.begin();
    save.initialize(savedData);
  }

  void update() {
    static unsigned long nextLoop = millis();
    while (millis() < nextLoop) delay(1);
    nextLoop += 33;

    loops++;
    now             = millis();
    curBtn          = input.update();
    curGnss.updated = (gnss.waitUpdate(0) == 1);
    if (curGnss.updated) gnss.getNavData(&curGnss.navData);

    if (curBtn != Input::Event::NONE) handleButton();
    trip.apply(TripData(trip.current(), curGnss, now));

    if (curBtn != Input::Event::NONE || now - lastUi >= Config::UI::UPDATE_INTERVAL_MS) {
      if (frame.apply(DisplayFrame(trip.current(), curGnss, trip.current().clock.now(), mode))) {
        renderer.render(frame.current());
        lastUi = now;
      }
    }

    if (now - lastSave >= DataStore::SAVE_INTERVAL_MS && !curGnss.updated) {
      if (save.apply(SaveData(trip.current(), batteryMonitor.update()))) store.save(save.current());
      lastSave = now;
    }

    if (now - lastFps >= 1000) {
      Serial.print("LOOPS: ");
      Serial.println(loops);
      loops   = 0;
      lastFps = now;
    }
  }

private:
  void handleButton() {
    auto &tripState = trip.current();

    switch (curBtn) {
    case Input::Event::SELECT:
      mode = static_cast<Mode>((static_cast<int>(mode) + 1) % 3);
      return;

    case Input::Event::PAUSE:
      tripState.togglePause();
      return;

    case Input::Event::RESET:
      if (mode == Mode::SPD_TIM) tripState.clearAvgOdo();
      if (mode == Mode::MAX_CLK) tripState.clearMaxSpeed();
      if (mode == Mode::AVG_ODO) tripState.clearAllData();
      return;

    case Input::Event::RESET_LONG:
      tripState.clearAllData();
      store.clear();
      renderer.resetDisplay();
      frame.initialize({});
      save.initialize(SaveData(tripState, 0));
      return;

    default:
      return;
    }
  }
};
