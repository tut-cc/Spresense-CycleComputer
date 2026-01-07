#include <gtest/gtest.h>
#include "Arduino.h"
#include "system/TripComputer.h"
#include "Config.h"

// Reset mocks before each test
class TripComputerTest : public ::testing::Test {
protected:
    void SetUp() override {
        _mock_millis = 1000;
    }

    void TearDown() override {
        // Cleanup if needed
    }
};

TEST_F(TripComputerTest, InitialState) {
    TripComputer tc;
    tc.begin();

    EXPECT_EQ(tc.getMaxSpeedKmh(), 0.0f);
    EXPECT_FLOAT_EQ(tc.getDistanceKm(), 0.0f);
}

TEST_F(TripComputerTest, CalculateDistance) {
    TripComputer tc;
    tc.begin();

    // 1st update: 0ms, 0km/h
    tc.update(0.0f, 0);

    // 2nd update: 1 hour later (3600000ms), moving at 10km/h
    // Note: TripComputer logic might depend on how it integrates.
    // Looking at source: distance += speed * time
    // If we jump from 0 to 1hr with constant speed, it should add 10km * 1hr = 10km
    
    // Simulate moving at 20km/h for 1 hour
    float speed = 20.0f;
    unsigned long duration = 3600000;
    unsigned long now = 1000;
    
    // Config::Trip::MOVE_THRESHOLD_KMH is 3.0f, so 20.0f is moving
    
    // Step 1: Start
    tc.update(speed, now);
    
    // Step 2: Update after 1 hour
    now += duration;
    tc.update(speed, now);

    EXPECT_NEAR(tc.getDistanceKm(), 20.0f, 0.01f);
    EXPECT_FLOAT_EQ(tc.getMaxSpeedKmh(), 20.0f);
}

TEST_F(TripComputerTest, IgnoreLowSpeed) {
    TripComputer tc;
    tc.begin();

    unsigned long now = 1000;
    
    // Speed below threshold (e.g. 1.0 km/h)
    float speed = 1.0f; 
    
    tc.update(speed, now);
    now += 3600000; // 1 hour
    tc.update(speed, now);

    // Should not accumulate distance
    EXPECT_FLOAT_EQ(tc.getDistanceKm(), 0.0f);
    // But max speed might update if logic allows (check source behavior)
    // Source: if (currentSpeedKmh > maxSpeedKmh) maxSpeedKmh = currentSpeedKmh;
    // So max speed updates regardless of movement threshold
    EXPECT_FLOAT_EQ(tc.getMaxSpeedKmh(), 1.0f);
}

TEST_F(TripComputerTest, TimeFormatting) {
    TripComputer tc;
    char buffer[16];
    
    // 1 hour, 1 minute, 1 second = 3600 + 60 + 1 = 3661 seconds = 3661000 ms
    unsigned long ms = 3661000;
    tc.msToTimeStr(ms, buffer, sizeof(buffer));
    
    EXPECT_STREQ(buffer, "01:01:01");
}
