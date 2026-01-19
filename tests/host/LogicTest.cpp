#include "logic/Clock.h"
#include "logic/DataStore.h"
#include "logic/Trip.h"
#include "logic/VoltageMonitor.h"
#include "mocks/Arduino.h"
#include "mocks/GNSS.h"
#include <gtest/gtest.h>
#include <limits>

// --- Trip Tests ---

class TripTest : public ::testing::Test {
protected:
  Trip trip;
  void SetUp() override {
    _mock_millis = 0;
    trip.begin();
  }
};

TEST_F(TripTest, InitialState) {
  auto state = trip.getState();
  EXPECT_FLOAT_EQ(state.currentSpeed, 0.0f);
  EXPECT_FLOAT_EQ(state.totalKm, 0.0f);
  EXPECT_EQ(state.status, Trip::Status::Stopped);
  EXPECT_EQ(state.totalMovingMs, 0);
}

TEST_F(TripTest, UpdateStatusMoving) {
  SpNavData navData;
  navData.velocity   = 10.0f / 3.6f; // 10 km/h
  navData.posFixMode = Fix3D;

  // First update to set baseline
  trip.update(navData, 1000, true);
  // Second update to calculate dt and update status to Moving
  trip.update(navData, 2000, true);

  EXPECT_EQ(trip.getState().status, Trip::Status::Moving);
  EXPECT_NEAR(trip.getState().currentSpeed, 10.0f, 0.01f);
}

TEST_F(TripTest, PauseToggle) {
  trip.pause();
  EXPECT_EQ(trip.getState().status, Trip::Status::Paused);
  trip.pause();
  EXPECT_EQ(trip.getState().status, Trip::Status::Stopped);
}

TEST_F(TripTest, AverageSpeed) {
  SpNavData navData;
  navData.velocity   = 36.0f / 3.6f; // 36 km/h
  navData.posFixMode = Fix3D;
  navData.latitude   = 35.6812;
  navData.longitude  = 139.7671;

  trip.update(navData, 1000, true); // sets hasLastUpdate

  trip.update(navData, 2000, true); // sets hasLastCoord, status becomes Moving

  // Move to another coordinate (approx 110m away)
  navData.latitude = 35.6822;
  trip.update(navData, 3000, true); // tripDistance increments, totalMovingMs increments (dt=1000)

  trip.update(navData, 4000, true); // additional stats update

  EXPECT_GT(trip.getState().tripDistance, 0.0f);
  EXPECT_GT(trip.getState().totalMovingMs, 0);
  EXPECT_GT(trip.getState().avgSpeed, 0.0f);
}

TEST_F(TripTest, GnssTimeout) {
  SpNavData navData;
  navData.velocity   = 10.0f / 3.6f;
  navData.posFixMode = Fix3D;

  trip.update(navData, 1000, true);
  trip.update(navData, 2000, true);
  EXPECT_EQ(trip.getState().status, Trip::Status::Moving);

  // Timeout
  trip.update(navData, 2000 + SIGNAL_TIMEOUT_MS + 100, false);
  EXPECT_EQ(trip.getState().status, Trip::Status::Stopped);
  EXPECT_FLOAT_EQ(trip.getState().currentSpeed, 0.0f);
}

TEST_F(TripTest, InvalidCoordinate) {
  SpNavData navData;
  navData.velocity   = 10.0f / 3.6f;
  navData.posFixMode = Fix3D;
  navData.latitude   = 35.0;
  navData.longitude  = 135.0;

  trip.update(navData, 1000, true);
  trip.update(navData, 2000, true);
  float initialDist = trip.getState().totalKm;

  // Update with (0,0)
  navData.latitude  = 0.0;
  navData.longitude = 0.0;
  trip.update(navData, 3000, true);
  EXPECT_FLOAT_EQ(trip.getState().totalKm, initialDist);
}

TEST_F(TripTest, ExtremeDistanceJump) {
  SpNavData navData;
  navData.velocity   = 10.0f / 3.6f;
  navData.posFixMode = Fix3D;
  navData.latitude   = 35.0;
  navData.longitude  = 135.0;

  trip.update(navData, 1000, true);
  trip.update(navData, 2000, true);
  float initialDist = trip.getState().totalKm;

  // Jump to another country (too far)
  navData.latitude  = 40.0;
  navData.longitude = 140.0;
  trip.update(navData, 3000, true);
  EXPECT_FLOAT_EQ(trip.getState().totalKm, initialDist);
}

TEST_F(TripTest, GnssFixLost) {
  SpNavData navData;
  navData.velocity   = 10.0f / 3.6f;
  navData.posFixMode = Fix3D;

  trip.update(navData, 1000, true);
  trip.update(navData, 2000, true);
  EXPECT_EQ(trip.getState().status, Trip::Status::Moving);

  // Lose fix
  navData.posFixMode = FixInvalid;
  trip.update(navData, 3000, true);
  EXPECT_EQ(trip.getState().status, Trip::Status::Stopped);
  EXPECT_FLOAT_EQ(trip.getState().currentSpeed, 0.0f);
}

TEST_F(TripTest, GnssJitter) {
  SpNavData navData;
  navData.velocity   = 10.0f / 3.6f;
  navData.posFixMode = Fix3D;
  navData.latitude   = 35.0;
  navData.longitude  = 135.0;

  trip.update(navData, 1000, true);
  trip.update(navData, 2000, true);
  float initialDist = trip.getState().totalKm;

  // Tiny movement (below MIN_DELTA = 2m)
  // 1 meter is approx 0.000009 degrees
  navData.latitude += 0.000005; // ~0.5 meters
  trip.update(navData, 3000, true);
  EXPECT_FLOAT_EQ(trip.getState().totalKm, initialDist);
}

TEST_F(TripTest, GnssFix2D) {
  SpNavData navData;
  navData.velocity   = 10.0f / 3.6f;
  navData.posFixMode = Fix2D; // Only 2D fix

  trip.update(navData, 1000, true);
  trip.update(navData, 2000, true);
  EXPECT_EQ(trip.getState().status, Trip::Status::Moving);
}

TEST_F(TripTest, MinMovingSpeed) {
  SpNavData navData;
  navData.posFixMode = Fix3D;

  // Just below threshold
  navData.velocity = (MIN_MOVING_SPEED_KMH - 0.0001f) / 3.6f;
  trip.update(navData, 1000, true);
  trip.update(navData, 2000, true);
  EXPECT_EQ(trip.getState().status, Trip::Status::Stopped);

  // Just above threshold
  navData.velocity = (MIN_MOVING_SPEED_KMH + 0.0001f) / 3.6f;
  trip.update(navData, 3000, true);
  EXPECT_EQ(trip.getState().status, Trip::Status::Moving);
}

TEST_F(TripTest, DistanceDeltaLimits) {
  SpNavData navData;
  navData.posFixMode = Fix3D;
  navData.velocity   = 10.0f / 3.6f;
  navData.latitude   = 35.0;
  navData.longitude  = 135.0;

  trip.update(navData, 1000, true);
  trip.update(navData, 2000, true); // hasLastCoord set

  float initialDist = trip.getState().totalKm;

  // Change coordinate by approx 3.3 meters (above 2m MIN_DELTA)
  navData.latitude += 0.00003;
  trip.update(navData, 3000, true);
  EXPECT_GT(trip.getState().totalKm, initialDist);
}

// --- Clock Tests ---

TEST(ClockTest, ValidTime) {
  SpNavData navData;
  navData.time = {2026, 1, 19, 10, 30, 0, 0}; // UTC 10:30

  Clock clock(navData);
  EXPECT_EQ(clock.hour, 19); // JST 19:30
  EXPECT_EQ(clock.minute, 30);
  EXPECT_EQ(clock.second, 0);
}

TEST(ClockTest, InvalidYear) {
  SpNavData navData;
  navData.time = {2023, 1, 1, 0, 0, 0, 0};

  Clock clock(navData);
  EXPECT_EQ(clock.hour, 0);
  EXPECT_EQ(clock.minute, 0);
  // Clock is initialized to 0s if year is invalid
}

TEST(ClockTest, TimeWrap) {
  SpNavData navData;
  navData.time = {2026, 1, 19, 20, 0, 0, 0}; // UTC 20:00

  Clock clock(navData);
  EXPECT_EQ(clock.hour, 5); // 20 + 9 = 29 -> 5 JST next day
}

// --- VoltageMonitor Tests ---

TEST(VoltageMonitorTest, LowVoltageAlert) {
  VoltageMonitor vm;
  vm.begin();
  float v = vm.update();
  EXPECT_GT(v, 0.0f);
}

// --- DataStore Tests ---

TEST(DataStoreTest, LoadSave) {
  DataStore ds;
  AppData   data = {100.5f, 10.2f, 3600000, 25.5f, 4.2f};
  ds.save(data);

  AppData loaded = ds.load();
  EXPECT_FLOAT_EQ(loaded.totalDistance, 100.5f);
  EXPECT_FLOAT_EQ(loaded.tripDistance, 10.2f);
  EXPECT_EQ(loaded.movingTimeMs, 3600000);
}

TEST(DataStoreTest, CorruptedCRC) {
  DataStore ds;
  AppData   data = {100.5f, 10.2f, 3600000, 25.5f, 4.2f};
  ds.save(data);

  // CRC is at offset 28
  EEPROM.buffer[28] ^= 0xFF;

  AppData loaded = ds.load();
  EXPECT_FLOAT_EQ(loaded.totalDistance, 0.0f); // Returns default on CRC fail
}

TEST(DataStoreTest, InvalidMagic) {
  DataStore ds;
  AppData   data = {100.5f, 10.2f, 3600000, 25.5f, 4.2f};
  ds.save(data);

  // Corrupt Magic Number (offset 0)
  uint32_t badMagic = 0x12345678;
  std::memcpy(&EEPROM.buffer[0], &badMagic, 4);

  // Note: we also need to fix CRC if we want to test magic number check independently,
  // but if CRC fails it already returns default.
  // Let's just corrupt magic and see it fails (either CRC or Magic check).
  AppData loaded = ds.load();
  EXPECT_FLOAT_EQ(loaded.totalDistance, 0.0f);
}

TEST(DataStoreTest, InvalidData) {
  DataStore ds;
  // Total Distance < 0 is invalid
  AppData data = {-10.0f, 10.2f, 3600000, 25.5f, 4.2f};

  // We need to bypass the 'save' logic if it prevents saving bad data,
  // but save() just calculates CRC and puts it.
  ds.save(data);

  AppData loaded = ds.load();
  EXPECT_FLOAT_EQ(loaded.totalDistance, 0.0f); // Validation should catch -10.0f
}

TEST(DataStoreTest, MaxDistanceLimit) {
  DataStore ds;
  // Exactly at limit
  AppData data = {MAX_VALID_KM, 0.0f, 0, 0.0f, 4.0f};
  ds.save(data);
  EXPECT_FLOAT_EQ(ds.load().totalDistance, MAX_VALID_KM);

  // Slightly over limit
  data.totalDistance = MAX_VALID_KM + 1.0f;
  ds.save(data);
  EXPECT_FLOAT_EQ(ds.load().totalDistance, 0.0f); // Should fail validation
}

TEST(DataStoreTest, Clear) {
  DataStore ds;
  AppData   data = {100.5f, 10.2f, 3600000, 25.5f, 4.2f};
  ds.save(data);

  // Verify data was saved
  AppData loaded = ds.load();
  EXPECT_FLOAT_EQ(loaded.totalDistance, 100.5f);

  // Clear the data
  ds.clear();
  AppData cleared = ds.load();

  // All values should be reset to 0
  EXPECT_FLOAT_EQ(cleared.totalDistance, 0.0f);
  EXPECT_FLOAT_EQ(cleared.tripDistance, 0.0f);
  EXPECT_EQ(cleared.movingTimeMs, 0);
  EXPECT_FLOAT_EQ(cleared.maxSpeed, 0.0f);
  EXPECT_FLOAT_EQ(cleared.voltage, 0.0f);
}

TEST(DataStoreTest, NaNDistance) {
  DataStore ds;
  // Save data with NaN totalDistance
  AppData data = {std::numeric_limits<float>::quiet_NaN(), 10.2f, 3600000, 25.5f, 4.2f};
  ds.save(data);

  // Load should return default values due to validation failure
  AppData loaded = ds.load();
  EXPECT_FLOAT_EQ(loaded.totalDistance, 0.0f);
  EXPECT_FLOAT_EQ(loaded.tripDistance, 0.0f);
}

TEST(DataStoreTest, VoltageOnlyChange) {
  DataStore ds;
  AppData   data = {100.5f, 10.2f, 3600000, 25.5f, 4.2f};
  ds.save(data);

  // Clear write count
  EEPROM.clearWriteCount();

  // Change only voltage (isDataEqual excludes voltage)
  data.voltage = 3.8f;
  ds.save(data);

  // No write should occur because isDataEqual returns true
  EXPECT_EQ(EEPROM.writeCount, 0);

  // Verify voltage change alone doesn't trigger save
  AppData loaded = ds.load();
  EXPECT_FLOAT_EQ(loaded.voltage, 4.2f); // Original voltage
}

TEST(DataStoreTest, InitialSave) {
  DataStore ds;
  // First save without any prior load
  AppData data = {50.0f, 5.0f, 1800000, 15.0f, 4.0f};

  EEPROM.clearWriteCount();
  ds.save(data);

  // Should write because lastSavedData is uninitialized
  EXPECT_GT(EEPROM.writeCount, 0);

  // Verify data was saved correctly
  AppData loaded = ds.load();
  EXPECT_FLOAT_EQ(loaded.totalDistance, 50.0f);
  EXPECT_FLOAT_EQ(loaded.tripDistance, 5.0f);
}

// --- AppData Tests ---

TEST(AppDataTest, EqualityOperator) {
  AppData data1 = {100.5f, 10.2f, 3600000, 25.5f, 4.2f};
  AppData data2 = {100.5f, 10.2f, 3600000, 25.5f, 4.2f};
  AppData data3 = {100.5f, 10.2f, 3600000, 25.5f, 3.8f}; // Different voltage

  // Same data should be equal
  EXPECT_TRUE(data1 == data2);
  EXPECT_FALSE(data1 != data2);

  // Different voltage should make them unequal
  EXPECT_FALSE(data1 == data3);
  EXPECT_TRUE(data1 != data3);
}

TEST(AppDataTest, IsDataEqual) {
  AppData data1 = {100.5f, 10.2f, 3600000, 25.5f, 4.2f};
  AppData data2 = {100.5f, 10.2f, 3600000, 25.5f, 3.8f}; // Different voltage
  AppData data3 = {100.6f, 10.2f, 3600000, 25.5f, 4.2f}; // Different totalDistance

  // isDataEqual should ignore voltage
  EXPECT_TRUE(data1.isDataEqual(data2));

  // isDataEqual should detect other differences
  EXPECT_FALSE(data1.isDataEqual(data3));
}

TEST(AppDataTest, IsDataEqualAllFields) {
  AppData base = {100.5f, 10.2f, 3600000, 25.5f, 4.2f};

  // Test each field individually
  AppData diffTotal   = {100.6f, 10.2f, 3600000, 25.5f, 4.2f};
  AppData diffTrip    = {100.5f, 10.3f, 3600000, 25.5f, 4.2f};
  AppData diffTime    = {100.5f, 10.2f, 3600001, 25.5f, 4.2f};
  AppData diffMaxSpd  = {100.5f, 10.2f, 3600000, 25.6f, 4.2f};
  AppData diffVoltage = {100.5f, 10.2f, 3600000, 25.5f, 3.8f};

  EXPECT_FALSE(base.isDataEqual(diffTotal));
  EXPECT_FALSE(base.isDataEqual(diffTrip));
  EXPECT_FALSE(base.isDataEqual(diffTime));
  EXPECT_FALSE(base.isDataEqual(diffMaxSpd));
  EXPECT_TRUE(base.isDataEqual(diffVoltage)); // Voltage is ignored
}
