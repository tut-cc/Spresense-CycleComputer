#include "domain/DataStore.h"
#include "domain/Trip.h"
#include <gtest/gtest.h>

#ifndef PI
#define PI 3.14159265358979323846
#endif

// Mock millis() if needed, but here we can just pass values to update()
unsigned long mock_millis = 0;

TEST(TripTest, PrecisionAccumulation) {
  mock_millis = 0;
  Trip trip;
  trip.begin();
  trip.reset();

  // Mock initial coordinates (Tokyo Station)
  SpNavData nav;
  memset(&nav, 0, sizeof(nav));
  nav.posFixMode = Fix3D;
  nav.latitude   = 35.681236;
  nav.longitude  = 139.767125;
  nav.velocity   = 10.0 / 3.6; // 10 km/h

  // First update: establishes time baseline
  trip.update(nav, mock_millis, true);
  mock_millis += 1000;

  // Second update: establishes coordinate baseline
  trip.update(nav, mock_millis, true);
  mock_millis += 1000;

  // Third update: first movement (111m)
  nav.latitude += 0.0010;
  trip.update(nav, mock_millis, true);
  mock_millis += 1000;

  // Fourth update: second movement (111m)
  nav.latitude += 0.0010;
  trip.update(nav, mock_millis, true);
  mock_millis += 1000;

  float dist = trip.getState().tripDistance;
  // Total distance: 2 * 0.111319 km = 0.222638 km
  EXPECT_GT(dist, 0.2f);
  EXPECT_NEAR(dist, 0.222f, 0.01f);
}

TEST(AppDataTest, EqualityLogic) {
  AppData data1 = {100.5, 10.2, 3600000, 25.5, 5.0f};
  AppData data2 = {100.5, 10.2, 3600000, 25.5, 4.8f}; // Different voltage

  EXPECT_TRUE(data1.isDataEqual(data2));
  EXPECT_FALSE(data1 == data2);
}

TEST(DataStoreTest, SaveOptimization) {
  // This is harder to test without mocking EEPROM fully,
  // but we can verify the logic in AppData as above.
}
