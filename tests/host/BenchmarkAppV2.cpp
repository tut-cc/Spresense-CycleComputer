#include "../../src2/App.h"
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

  const int N = 1000000;

  std::cout << "Starting Benchmark V2 (src2) - " << N << " iterations..." << std::endl;

  auto start = std::chrono::high_resolution_clock::now();

  for (int i = 1; i <= N; ++i) {
    _mock_millis = i;
    app.update();
  }

  auto                          end  = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> diff = end - start;

  std::cout << std::fixed << std::setprecision(6);
  std::cout << "Results for src2/App:" << std::endl;
  std::cout << "Total Time: " << diff.count() << " s" << std::endl;
  std::cout << "Avg Time/It: " << (diff.count() * 1e6 / N) << " us" << std::endl;

  return 0;
}
