#pragma once

#include "logic/Trip.h"
#include "mocks/Arduino.h"
#include "mocks/GNSS.h"
#include <gtest/gtest.h>

/**
 * @brief Trip関連のテストで共通して使用するフィクスチャ
 */
class TripTestBase : public ::testing::Test {
protected:
  Trip      trip;
  SpNavData navData;

  void SetUp() override {
    _mock_millis = 0;
    _mock_pin_states.clear();
    _mock_analog_values.clear();
    trip.begin();

    // デフォルトの有効データ
    navData.posFixMode = Fix3D;
    navData.velocity   = 10.0f / 3.6f; // 10 km/h
    navData.latitude   = 35.0f;
    navData.longitude  = 135.0f;
  }

  /**
   * @brief Tripの状態を「移動中」にするための共通ステップ
   */
  void setupMovingState(unsigned long startMs = 1000) {
    updateTrip(startMs);       // hasLastUpdate = true
    updateTrip(startMs + 100); // hasLastCoord = true, status -> Moving
  }

  void updateTrip(unsigned long ms, bool updated = true) {
    trip.update(navData, ms, updated);
  }
};
