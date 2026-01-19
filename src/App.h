#include <Arduino.h>
#include <stddef.h>

#include "hardware/Gnss.h"
#include "logic/Clock.h"
#include "logic/DataStore.h"
#include "logic/Trip.h"
#include "logic/VoltageMonitor.h"
#include "ui/UI.h"

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
      AppData            currentData;
      const Trip::State &state  = trip.getState();
      currentData.totalDistance = state.totalKm;
      currentData.tripDistance  = state.tripDistance;
      currentData.movingTimeMs  = state.totalMovingMs;
      currentData.maxSpeed      = state.maxSpeed;
      currentData.voltage       = currentVoltage;

      dataStore.save(currentData);
      lastSaveMillis = millis();
    }
  }
};

class App {
private:
  Gnss gnss;
  Trip trip;

  DataStore dataStore;

  VoltageMonitor  voltageMonitor;
  DataPersistence dataPersistence;
  UI              userInterface;

public:
  App() : dataPersistence(dataStore, trip) {}

  void begin() {
    gnss.begin();
    trip.begin();
    voltageMonitor.begin();
    dataPersistence.load();
    userInterface.begin();
  }

  void update() {
    const bool      isGnssUpdated = gnss.update();
    const SpNavData navData       = gnss.getNavData();

    trip.update(navData, millis(), isGnssUpdated);
    Clock clock(navData);

    float currentVoltage = voltageMonitor.update();
    dataPersistence.update(isGnssUpdated, currentVoltage);
    userInterface.update(trip, dataStore, clock, navData);
  }
};
