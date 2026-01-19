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
  const int iterations      = 10000;
  long long max_ns          = 0;
  long long total_ns        = 0;
  int       spike_iteration = -1;

  for (int i = 0; i < iterations; ++i) {
    _mock_millis += 10; // Advance 10ms each loop

    auto start = std::chrono::high_resolution_clock::now();
    app.update();
    auto end = std::chrono::high_resolution_clock::now();

    long long duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    total_ns += duration;
    if (duration > max_ns) {
      max_ns          = duration;
      spike_iteration = i;
    }

    // Reset max intermittently to catch different spikes
    if (i == 5000) {
      std::cout << "[ PROFILE  ] Max in first 5000: " << max_ns << " ns at i=" << spike_iteration
                << std::endl;
      max_ns          = 0;
      spike_iteration = -1; // Reset spike iteration for the second half
    }

    // Periodically trigger a save (every 6000 iterations = 60s)
    // or trigger trip start to see persistence save
    if (i == 100) {
      // Force a trip move to trigger first save
      SpGnss::mockVelocityData = 10.0f;
    }
  }

  std::cout << "[ PROFILE  ] Average loop time: " << (total_ns / iterations) << " ns" << std::endl;
  std::cout << "[ PROFILE  ] Max in second 5000: " << max_ns << " ns at i=" << spike_iteration
            << std::endl;

  // Check if max is significantly higher than average (suggesting a spike)
  // In host environment, OS jitter might cause this, but let's see.
}
