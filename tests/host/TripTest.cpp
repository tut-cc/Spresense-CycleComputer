#include "system/Trip.h"
#include "../mocks/Arduino.h"
#include <gtest/gtest.h>

class TripTest : public ::testing::Test {
protected:
  application::Trip trip;

  void SetUp() override {
    trip.begin();
  }

  SpNavData createNavData(float speedKmh) {
    SpNavData data;
    data.velocity   = speedKmh / 3.6f;
    data.posFixMode = Fix3D;
    data.time       = {2021, 1, 1, 12, 0, 0, 0}; // Dummy time
    return data;
  }
};

TEST_F(TripTest, InitialState) {
  EXPECT_FLOAT_EQ(trip.getDistanceKm(), 0.0f);
  EXPECT_FLOAT_EQ(trip.getMaxSpeedKmh(), 0.0f);
  EXPECT_FLOAT_EQ(trip.getAvgSpeedKmh(), 0.0f);
}

TEST_F(TripTest, CalculateDistance) {
  // 36 km/h = 10 m/s = 0.01 km/s
  // 1 hour = 3600000 ms
  // 36 km/h for 1 hour -> 36 km

  unsigned long now = 1000;
  trip.update(createNavData(0.0f), now); // Init start time

  now += 3600000; // +1 hour
  float speed = 36.0f;
  trip.update(createNavData(speed), now);

  EXPECT_NEAR(trip.getDistanceKm(), 36.0f, 0.01f);
}

TEST_F(TripTest, IgnoreLowSpeed) {
  unsigned long now = 1000;
  trip.update(createNavData(0.0f), now);

  now += 3600000;
  float speed = 2.0f; // Below 3.0 km/h threshold
  trip.update(createNavData(speed), now);

  EXPECT_FLOAT_EQ(trip.getDistanceKm(), 0.0f);
}

TEST_F(TripTest, TimeFormatting) {
  char buffer[32];

  // Test 0
  trip.getMovingTimeStr(buffer, sizeof(buffer));
  EXPECT_STREQ(buffer, "00:00");

  // Test 1h 1m 1s = 3600 + 60 + 1 = 3661 sec = 3661000 ms
  // Manually injecting moving time via update logic is hard without specific speed profile.
  // But Trip delegates directly to Stopwatch using calculated movingTime.
  // Let's simulate movement.

  unsigned long now = 1000;
  trip.update(createNavData(0.0f), now);

  now += 3661000;
  trip.update(createNavData(10.0f), now); // Moving, so adds to moving time

  trip.getMovingTimeStr(buffer, sizeof(buffer));
  EXPECT_STREQ(buffer, "01:01");
}

TEST_F(TripTest, CalculateAvgSpeed) {
  unsigned long now = 1000;
  trip.update(createNavData(0.0f), now);

  now += 3600000;                         // 1 hour
  trip.update(createNavData(60.0f), now); // 60 km/h for 1h -> 60km distance, 1h moving time

  EXPECT_NEAR(trip.getAvgSpeedKmh(), 60.0f, 0.1f);
}

TEST_F(TripTest, AvgSpeedWithNoMovement) {
  EXPECT_FLOAT_EQ(trip.getAvgSpeedKmh(), 0.0f);
}

TEST_F(TripTest, Reset) {
  unsigned long now = 1000;
  trip.update(createNavData(0.0f), now);

  now += 1000;
  trip.update(createNavData(10.0f), now);

  EXPECT_GT(trip.getDistanceKm(), 0.0f);
  EXPECT_GT(trip.getMaxSpeedKmh(), 0.0f);

  trip.reset();

  EXPECT_FLOAT_EQ(trip.getDistanceKm(), 0.0f);
  EXPECT_FLOAT_EQ(trip.getMaxSpeedKmh(), 0.0f);
}

TEST_F(TripTest, StopMovingDoesNotIncreaseMovingTime) {
  unsigned long now = 1000;
  trip.update(createNavData(0.0f), now);

  now += 3600000;                         // 1 hour
  trip.update(createNavData(10.0f), now); // Moving for 1 hour
  // movingTime = 1h

  char buffer[32];
  trip.getMovingTimeStr(buffer, sizeof(buffer));
  // 1h -> 01:00
  EXPECT_STREQ(buffer, "01:00");

  now += 3600000;                        // Another hour passes...
  trip.update(createNavData(0.0f), now); // ...but not moving
  // movingTime should stay 1h

  trip.getMovingTimeStr(buffer, sizeof(buffer));
  EXPECT_STREQ(buffer, "01:00");
}
