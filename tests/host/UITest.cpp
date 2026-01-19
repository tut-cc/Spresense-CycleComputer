#include "ui/UI.h"
#include "mocks/Arduino.h"
#include "ui/Input.h"
#include "ui/Mode.h"
#include "ui/Renderer.h"
#include <gtest/gtest.h>

// --- Mode Tests ---

TEST(ModeTest, NextMode) {
  Mode      mode;
  Trip      trip;
  DataStore dataStore;

  // Initial mode is SPD_TIM (0)
  // SELECT button should cycle modes
  mode.handleInput(Input::Event::SELECT, trip, dataStore);
  // Becomes AVG_ODO

  Frame     frame;
  SpNavData navData;
  navData.time = {2026, 1, 19, 10, 30, 0, 0};
  Clock clock(navData);
  mode.fillFrame(frame, trip, clock);
  EXPECT_STREQ(frame.header.modeSpeed, "AVG");
}

TEST(ModeTest, ResetTrip) {
  Mode      mode;
  Trip      trip;
  DataStore dataStore;

  SpNavData navData;
  navData.velocity   = 10.0f;
  navData.posFixMode = Fix3D;
  trip.update(navData, 1000, true);
  trip.update(navData, 2000, true);

  EXPECT_GT(trip.getState().currentSpeed, 0.0f);

  mode.handleInput(Input::Event::RESET, trip, dataStore);
  EXPECT_FLOAT_EQ(trip.getState().tripDistance, 0.0f);
}

// --- Formatter Tests ---

TEST(FormatterTest, FormatDuration) {
  char buffer[16];
  Formatter::formatDuration(3661000, buffer, sizeof(buffer)); // 1h 1m 1s
  EXPECT_STREQ(buffer, "1:01:01");

  Formatter::formatDuration(61000, buffer, sizeof(buffer)); // 1m 1s
  EXPECT_STREQ(buffer, "01:01");
}

TEST(FormatterTest, FormatSpeed) {
  char buffer[16];
  Formatter::formatSpeed(12.34f, buffer, sizeof(buffer));
  EXPECT_STREQ(buffer, "12.3"); // %4.1f
}

// --- Input Tests ---

TEST(InputTest, SinglePress) {
  _mock_millis = 0;
  _mock_pin_states.clear();
  Input input(PIN_D02, PIN_D03);
  input.begin();

  _mock_pin_states[PIN_D02] = LOW;
  input.update();
  _mock_millis = 100;
  input.update();

  _mock_millis       = 200;
  Input::Event event = input.update();

  EXPECT_EQ(event, Input::Event::SELECT);
}

// --- UI Tests ---

TEST(UITest, InitialState) {
  UI ui;
  ui.begin();
}
