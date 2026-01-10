#include "system/TripComputer.h"
#include "../mocks/Arduino.h"
#include <gtest/gtest.h>

// Fixture for TripComputer tests
class TripComputerTest : public ::testing::Test {
protected:
  application::TripComputer tripComputer;

  void SetUp() override {
    _mock_millis = 1000;
    tripComputer.begin();
  }
};

TEST_F(TripComputerTest, InitialState) {
  EXPECT_FLOAT_EQ(tripComputer.getDistanceKm(), 0.0f);
  EXPECT_FLOAT_EQ(tripComputer.getMaxSpeedKmh(), 0.0f);
  EXPECT_FLOAT_EQ(tripComputer.getAvgSpeedKmh(), 0.0f);

  char buf[20];
  tripComputer.getMovingTimeStr(buf, sizeof(buf));
  EXPECT_STREQ(buf, "00:00:00");
}

TEST_F(TripComputerTest, CalculateDistance) {
  // Speed 36 km/h = 10 m/s
  // 1 hour = 3600 sec = 3600000 ms
  // Distance should be 36 km

  // Update 1: Start at 1000
  tripComputer.update(36.0f, 1000);

  // Update 2: After 1 hour (1000 + 3600000)
  tripComputer.update(36.0f, 3601000);

  EXPECT_NEAR(tripComputer.getDistanceKm(), 36.0f, 0.01f);
}

TEST_F(TripComputerTest, IgnoreLowSpeed) {
  // Speed 2 km/h (below threshold)
  tripComputer.update(2.0f, 1000);
  tripComputer.update(2.0f, 3601000);

  EXPECT_FLOAT_EQ(tripComputer.getDistanceKm(), 0.0f);

  char buf[20];
  tripComputer.getMovingTimeStr(buf, sizeof(buf));
  EXPECT_STREQ(buf, "00:00:00");
}

TEST_F(TripComputerTest, TimeFormatting) {
  // 1 hour, 1 minute, 1 second = 3600 + 60 + 1 = 3661 seconds = 3661000 ms
  char buf[20];
  tripComputer.msToTimeStr(3661000, buf, sizeof(buf));
  EXPECT_STREQ(buf, "01:01:01");
}

TEST_F(TripComputerTest, CalculateAvgSpeed) {
  // 36 km in 1 hour moving time
  tripComputer.update(36.0f, 1000);
  tripComputer.update(36.0f, 3601000); // 1 hour elapsed

  EXPECT_NEAR(tripComputer.getAvgSpeedKmh(), 36.0f, 0.1f);
}

TEST_F(TripComputerTest, AvgSpeedWithNoMovement) {
  EXPECT_FLOAT_EQ(tripComputer.getAvgSpeedKmh(), 0.0f);
}

TEST_F(TripComputerTest, HelperTimeStrings) {
  // Simulate 1 hour moving
  tripComputer.update(10.0f, 1000);
  tripComputer.update(10.0f, 3601000);

  char buf[20];
  tripComputer.getMovingTimeStr(buf, sizeof(buf));
  EXPECT_STREQ(buf, "01:00:00");

  // Correctly setting mock millis for getElapsedTimeStr (3601000)
  _mock_millis = 3601000;
  // Note: TripComputer stores startTimeMs declared in SetUp -> begin() which calls reset().
  // reset() sets startTimeMs = millis().
  // In SetUp (new), _mock_millis = 1000. So startTimeMs = 1000.
  // We want elapsed = 3600000. So current = 3601000.

  tripComputer.getElapsedTimeStr(buf, sizeof(buf));
  EXPECT_STREQ(buf, "01:00:00");
}

TEST_F(TripComputerTest, Reset) {
  tripComputer.update(36.0f, 1000);
  tripComputer.update(36.0f, 3601000);

  EXPECT_GT(tripComputer.getDistanceKm(), 0);

  tripComputer.reset();

  EXPECT_FLOAT_EQ(tripComputer.getDistanceKm(), 0.0f);
  EXPECT_FLOAT_EQ(tripComputer.getMaxSpeedKmh(), 0.0f);
}

TEST_F(TripComputerTest, StopMovingDoesNotIncreaseMovingTime) {
  unsigned long now = 1000;
  // Move for 1 hour
  tripComputer.update(36.0f, now);
  now += 3600000;
  tripComputer.update(36.0f, now);

  // Stop for 1 hour
  // Speed 0
  unsigned long stopStart = now;
  now += 3600000;
  tripComputer.update(0.0f, now);

  char buf[20];
  tripComputer.getMovingTimeStr(buf, sizeof(buf));
  EXPECT_STREQ(buf, "01:00:00"); // Should still be 1 hour

  // Elaborate elapsed time check
  _mock_millis = now;
  // startTime was 1000. Now is 1000 + 3600000 + 3600000 = 7201000.
  // Elapsed = 7200000 = 2 hours.
  tripComputer.getElapsedTimeStr(buf, sizeof(buf));
  EXPECT_STREQ(buf, "02:00:00");
}
