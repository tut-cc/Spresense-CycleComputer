#include "../../src2/App.h"
#include "mocks/Arduino.h"
#include "mocks/GNSS.h"
#include <chrono>
#include <gtest/gtest.h>
#include <iostream>

// --- App2 (Pipeline) Tests ---

class App2Test : public ::testing::Test {
protected:
  App app;

  void SetUp() override {
    _mock_millis             = 0;
    _mock_pin_states[BTN_A]  = HIGH;
    _mock_pin_states[BTN_B]  = HIGH;
    SpGnss::mockVelocityData = 0.0f;
    app.begin();
  }

  void TearDown() override {}
};

TEST_F(App2Test, Initialization) {
  // Successful start
}

TEST_F(App2Test, MainLoop) {
  _mock_millis = 1000;
  app.update();
}

TEST_F(App2Test, LoopProfiling) {
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

  std::cout << "[ PROFILE  ] App2 (Pipeline) Average loop time: " << (total_ns / iterations)
            << " ns" << std::endl;
}
