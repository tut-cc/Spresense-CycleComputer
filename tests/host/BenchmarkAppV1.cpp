#include "../../src/App.h"
#include "mocks/Arduino.h"
#include <chrono>
#include <iomanip>
#include <iostream>

// Mock millis defined in mocks
extern unsigned long      _mock_millis;
extern std::map<int, int> _mock_pin_states;

int main() {
  // Setup mocks
  _mock_millis = 0;

  App app;
  app.begin();

  const int iterations = 100000; // 100k iterations
  // Note: Reduced from 1M to 100k for safety within tool timeout,
  // but can increase if fast enough. 1M might take a few seconds which is fine.
  // Let's stick to 100,000 to be safely fast, and extrap if needed.
  // Actually 100,000 might be too fast to measure?
  // Benchmark.cpp used 1,000,000. Let's use 1,000,000.

  const int N = 1000000;

  std::cout << "Starting Benchmark V1 (src) - " << N << " iterations..." << std::endl;

  auto start = std::chrono::high_resolution_clock::now();

  for (int i = 1; i <= N; ++i) {
    _mock_millis = i;
    app.update();
  }

  auto                          end  = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> diff = end - start;

  std::cout << std::fixed << std::setprecision(6);
  std::cout << "Results for src/App:" << std::endl;
  std::cout << "Total Time: " << diff.count() << " s" << std::endl;
  std::cout << "Avg Time/It: " << (diff.count() * 1e6 / N) << " us" << std::endl;

  return 0;
}
