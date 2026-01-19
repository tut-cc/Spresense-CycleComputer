#include "../../src/logic/Trip.h"
#include "../../src2/DataStructures.h"
#include "../../src2/TripCompute.h"
#include "mocks/Arduino.h"
#include "mocks/GNSS.h"
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>

// Mock millis defined in mocks
extern unsigned long _mock_millis;

int main() {
  const int iterations = 1000000;

  // Test data
  SpNavData navData;
  navData.velocity   = 20.0f / 3.6f; // 20 km/h
  navData.posFixMode = Fix3D;
  navData.latitude   = 35.6812;
  navData.longitude  = 139.7671;
  navData.time.year  = 2026;

  GnssData gnssData;
  gnssData.navData   = navData;
  gnssData.timestamp = 0;
  gnssData.status    = UpdateStatus::Updated;

  std::cout << "Starting Benchmark (" << iterations << " iterations)..." << std::endl;

  // --- src (v1) ---
  Trip trip;
  trip.begin();
  auto start1 = std::chrono::high_resolution_clock::now();
  for (int i = 1; i <= iterations; ++i) {
    _mock_millis = i;
    trip.update(navData, i, (i % 10 == 0)); // Update GNSS every 10ms
    if (i % 10 == 0) { navData.latitude += 0.000001f; }
  }
  auto                          end1  = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> diff1 = end1 - start1;

  // Reset
  navData.latitude = 35.6812;

  // --- src2 (v2) ---
  TripStateDataEx state;
  state.resetAll();
  auto start2 = std::chrono::high_resolution_clock::now();
  for (int i = 1; i <= iterations; ++i) {
    _mock_millis       = i;
    gnssData.navData   = navData;
    gnssData.timestamp = i;
    gnssData.status    = (i % 10 == 0) ? UpdateStatus::Updated : UpdateStatus::NoChange;

    Pipeline::computeTrip(state, gnssData, i);

    if (i % 10 == 0) { navData.latitude += 0.000001f; }
  }
  auto                          end2  = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> diff2 = end2 - start2;

  std::cout << std::fixed << std::setprecision(6);
  std::cout << "\n--- Performance Results ---" << std::endl;
  std::cout << "src (v1)  : " << diff1.count() << " s (" << (diff1.count() * 1e6 / iterations)
            << " us/it)" << std::endl;
  std::cout << "src2 (v2) : " << diff2.count() << " s (" << (diff2.count() * 1e6 / iterations)
            << " us/it)" << std::endl;
  std::cout << "Speedup   : " << (diff1.count() / diff2.count()) << "x" << std::endl;

  std::cout << "\n--- Accuracy Check ---" << std::endl;
  std::cout << "src (v1) Distance: " << trip.getState().totalKm << " km" << std::endl;
  std::cout << "src2 (v2) Distance: " << state.totalKm << " km" << std::endl;
  std::cout << "Diff             : " << std::abs(trip.getState().totalKm - state.totalKm) << " km"
            << std::endl;

  return 0;
}
