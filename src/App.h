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

class VoltageMonitor {
private:
  VoltageSensor voltageSensor;

public:
  VoltageMonitor() : voltageSensor(Pin::VOLTAGE_PIN) {}

  void begin() {
    voltageSensor.begin();
    pinMode(Pin::WARN_LED, OUTPUT);
  }

  float update() {
    const float currentVoltage = voltageSensor.readVoltage();
    if (currentVoltage <= Battery::LOW_VOLTAGE_THRESHOLD) {
      digitalWrite(Pin::WARN_LED, HIGH);
    } else {
      digitalWrite(Pin::WARN_LED, LOW);
    }
    return currentVoltage;
  }
};

class DataPersistence {
private:
  DataStore    &dataStore;
  Trip         &trip;
  unsigned long lastSaveMillis = 0;

public:
  DataPersistence(DataStore &ds, Trip &t) : dataStore(ds), trip(t) {}

  void load() {
    AppData savedData = dataStore.load();
    trip.restore(savedData.totalDistance, savedData.tripDistance, savedData.movingTimeMs,
                 savedData.maxSpeed);
    lastSaveMillis = millis();
  }

  void update(bool isGnssUpdated, float currentVoltage) {
    if ((millis() - lastSaveMillis > DataStore::SAVE_INTERVAL_MS) && !isGnssUpdated) {
      AppData currentData;
      currentData.totalDistance  = trip.getTotalDistance();
      currentData.tripDistance   = trip.getTripDistance();
      currentData.movingTimeMs   = trip.getTotalMovingTimeMs();
      currentData.maxSpeed       = trip.getMaxSpeed();
      currentData.batteryVoltage = currentVoltage;

      dataStore.save(currentData);
      lastSaveMillis = millis();
    }
  }
};

class UserInterface {
private:
  OLED     oled;
  Input    input;
  Mode     mode;
  Renderer renderer;

  Frame createFrame(const SpNavData &navData, const Trip &trip, const Clock &clock) const {
    Frame frame;

    switch (navData.posFixMode) {
    case Fix2D:
      strcpy(frame.header.fixStatus, "2D");
      break;
    case Fix3D:
      strcpy(frame.header.fixStatus, "3D");
      break;
    default:
      strcpy(frame.header.fixStatus, "WAIT");
      break;
    }

    mode.getCurrentState()->fillFrame(frame, trip, clock);

    return frame;
  }

public:
  UserInterface() : oled(OLED::WIDTH, OLED::HEIGHT), input(Pin::BTN_A, Pin::BTN_B) {}

  void begin() {
    oled.begin(OLED::ADDRESS);
    input.begin();
  }

  void update(Trip &trip, DataStore &dataStore, const Clock &clock, const SpNavData &navData) {
    Input::ID id = input.update();

    if (id == Input::ID::RESET_LONG) {
      oled.restart();
      renderer.reset();
    }

    if (id != Input::ID::NONE) { mode.handleInput(id, trip, dataStore); }

    Frame frame = createFrame(navData, trip, clock);
    renderer.render(oled, frame);
  }
};

class App {
private:
  Gnss      gnss;
  Trip      trip;
  Clock     clock;
  DataStore dataStore;

  VoltageMonitor  batteryMonitor;
  DataPersistence dataPersistence;
  UserInterface   userInterface;

public:
  App() : dataPersistence(dataStore, trip) {}

  void begin() {
    gnss.begin();
    trip.begin();
    batteryMonitor.begin();
    dataPersistence.load();
    userInterface.begin();
  }

  void update() {
    const bool      isGnssUpdated = gnss.update();
    const SpNavData navData       = gnss.getNavData();

    trip.update(navData, millis());
    clock.update(navData);

    float currentVoltage = batteryMonitor.update();
    dataPersistence.update(isGnssUpdated, currentVoltage);
    userInterface.update(trip, dataStore, clock, navData);
  }
};
