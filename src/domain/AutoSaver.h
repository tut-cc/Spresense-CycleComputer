#pragma once

#include "DataStore.h"
#include "Trip.h"

class AutoSaver {
private:
  DataStore &dataStore;
  Trip      &trip;

  unsigned long lastSaveMillis = 0;

public:
  AutoSaver(DataStore &ds, Trip &t) : dataStore(ds), trip(t) {}

  void begin() {
    lastSaveMillis = millis();
  }

  void update(unsigned long currentMillis) {
    if (currentMillis - lastSaveMillis > DataStore::SAVE_INTERVAL_MS) {
      AppData currentData;
      currentData.totalDistance = trip.odometer.getTotalDistance();
      currentData.tripDistance  = trip.getTripDistance();
      currentData.movingTimeMs  = trip.getMovingTimeMs();

      dataStore.save(currentData);
      lastSaveMillis = currentMillis;
    }
  }
};
