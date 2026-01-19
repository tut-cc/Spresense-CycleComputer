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
  const int iterations = 10000;
  long long total_ns   = 0;

  for (int i = 0; i < iterations; ++i) {
    _mock_millis += 10;
    auto start = std::chrono::high_resolution_clock::now();
    app.update();
    auto end = std::chrono::high_resolution_clock::now();
    total_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  }

  std::cout << "[ PROFILE  ] App2 (Pipeline) Average loop time: " << (total_ns / iterations)
            << " ns" << std::endl;
}
