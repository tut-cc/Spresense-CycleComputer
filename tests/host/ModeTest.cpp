#include "system/Mode.h"
#include <gtest/gtest.h>

class ModeTest : public ::testing::Test {
protected:
  application::Mode mode; // Renamed from modeManager
};

TEST_F(ModeTest, InitialModeIsSpeed) {
  EXPECT_EQ(mode.get(), application::Mode::ID::SPEED);
}

TEST_F(ModeTest, CycleThroughModes) {
  // Sequence: SPEED -> MAX_SPEED -> AVG_SPEED -> TIME -> MOVING_TIME -> ELAPSED_TIME -> DISTANCE -> SPEED

  mode.next();
  EXPECT_EQ(mode.get(), application::Mode::ID::MAX_SPEED);

  mode.next();
  EXPECT_EQ(mode.get(), application::Mode::ID::AVG_SPEED);

  mode.next();
  EXPECT_EQ(mode.get(), application::Mode::ID::TIME);

  mode.next();
  EXPECT_EQ(mode.get(), application::Mode::ID::MOVING_TIME);

  mode.next();
  EXPECT_EQ(mode.get(), application::Mode::ID::ELAPSED_TIME);

  mode.next();
  EXPECT_EQ(mode.get(), application::Mode::ID::DISTANCE);

  mode.next();
  EXPECT_EQ(mode.get(), application::Mode::ID::SPEED);
}
