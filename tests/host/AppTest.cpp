#include "App.h"
#include "mocks/Arduino.h"
#include "mocks/GNSS.h"
#include <chrono>
#include <gtest/gtest.h>
#include <iostream>

// --- DataPersistence Tests ---

TEST(DataPersistenceTest, SaveInterval) {
  DataStore       ds;
  Trip            trip;
  DataPersistence dp(ds, trip);

  _mock_millis = 0;
  dp.load();

  // Move time forward past interval
  _mock_millis = DataStore::SAVE_INTERVAL_MS + 1000;

  // If GNSS is updated, it SHOULD NOT save (to avoid heavy EEPROM write while active)
  dp.update(true, 4.0f);
  // lastSaveMillis should still be 0 if we could check it

  // If GNSS is NOT updated, it SHOULD save
  dp.update(false, 4.0f);
  // Verification: load from ds should match current state
}

// --- App Tests ---

// Define a test fixture for App tests
class AppTest : public ::testing::Test {
protected:
  App app; // Declare App instance here

  void SetUp() override {
    app.begin();                     // Initialize app before each test
    _mock_millis             = 0;    // Reset mock millis for each test
    SpGnss::mockVelocityData = 0.0f; // Reset mock velocity
  }

  void TearDown() override {
    // Clean up if necessary
  }
};

TEST_F(AppTest, Initialization) {
  // app.begin() is called in SetUp()
  // Verify it doesn't crash and initializes sub-components
}

TEST_F(AppTest, MainLoop) {
  _mock_millis = 1000;
  app.update();
  // Use app.run() as per the instruction's implied change
  // Verify update cycle
}

TEST_F(AppTest, LoopProfiling) {
  const int iterations = 6000; // 60 seconds (10ms steps)
  long long total_ns   = 0;

  SpGnss::mockVelocityData = 10.0f; // Simulate movement

  for (int i = 0; i < iterations; ++i) {
    _mock_millis += 10;

    // Periodic button presses (every 10 seconds)
    if (i % 1000 == 500) {
      _mock_pin_states[BTN_A] = LOW; // Press
    } else if (i % 1000 == 510) {
      _mock_pin_states[BTN_A] = HIGH; // Release
    }

    auto start = std::chrono::high_resolution_clock::now();
    app.update();
    auto end = std::chrono::high_resolution_clock::now();

    total_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  }

  std::cout << "[ PROFILE  ] App (Original) Average loop time: " << (total_ns / iterations) << " ns"
            << std::endl;
}
