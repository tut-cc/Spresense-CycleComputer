#pragma once

#include <stddef.h>

#include "Config.h"
#include "domain/Clock.h"
#include "domain/DataStore.h"
#include "domain/Trip.h"
#include "hardware/Gnss.h"
#include "hardware/OLED.h"
#include "hardware/VoltageSensor.h"

#include "ui/Input.h"
#include "ui/Mode.h"
#include "ui/Renderer.h"

class App {
private:
  OLED          oled;
  Input         input;
  Gnss          gnss;
  Mode          mode;
  Trip          trip;
  Clock         clock;
  Renderer      renderer;
  DataStore     dataStore;
  VoltageSensor voltageSensor;
  unsigned long lastSaveMillis = 0;

public:
  App()
      : oled(OLED::WIDTH, OLED::HEIGHT), input(Pin::BTN_A, Pin::BTN_B),
        voltageSensor(Pin::VOLTAGE_PIN) {}

  void begin() {
    oled.begin(OLED::ADDRESS);
    input.begin();
    gnss.begin();
    trip.begin();
    voltageSensor.begin();
    pinMode(Pin::WARN_LED, OUTPUT);

    AppData savedData = dataStore.load();

    trip.restore(savedData.totalDistance, savedData.tripDistance, savedData.movingTimeMs);

    lastSaveMillis = millis();
  }

  void update() {
    handleInput();

    gnss.update();
    const NavData navData = gnss.getNavData();

    trip.update(navData, millis());
    clock.update(navData);

    const float currentVoltage = voltageSensor.readVoltage();
    if (currentVoltage <= Battery::LOW_VOLTAGE_THRESHOLD) {
      digitalWrite(Pin::WARN_LED, HIGH);
    } else {
      digitalWrite(Pin::WARN_LED, LOW);
    }

    if (millis() - lastSaveMillis > DataStore::SAVE_INTERVAL_MS) {
      AppData currentData;
      currentData.totalDistance  = trip.getTotalDistance();
      currentData.tripDistance   = trip.getTripDistance();
      currentData.movingTimeMs   = trip.getMovingTimeMs();
      currentData.batteryVoltage = currentVoltage;

      dataStore.save(currentData);
      lastSaveMillis = millis();
    }

    Frame frame = createFrame(navData);
    renderer.render(oled, frame);
  }

private:
  Frame createFrame(const NavData &navData) const {
    Frame frame;

    switch (navData.fixType) {
    case FixType::NoFix:
      strcpy(frame.header.fixStatus, "WAIT");
      break;
    case FixType::Fix2D:
      strcpy(frame.header.fixStatus, "2D");
      break;
    case FixType::Fix3D:
      strcpy(frame.header.fixStatus, "3D");
      break;
    default:
      strcpy(frame.header.fixStatus, "");
      break;
    }

    mode.getCurrentState()->fillFrame(frame, trip, clock);

    return frame;
  }

  void handleInput() {
    Input::ID id = input.update();
    if (id != Input::ID::NONE) { mode.handleInput(id, trip, dataStore); }
  }
};
