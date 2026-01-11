#include "ui/Mode.h"
#include <gtest/gtest.h>

class ModeTest : public ::testing::Test {
protected:
  ui::Mode mode; // Renamed from modeManager
};

TEST_F(ModeTest, InitialModeIsSpeed) {
  EXPECT_EQ(mode.get(), ui::Mode::ID::SPEED);
}

TEST_F(ModeTest, CycleThroughModes) {
  // Sequence: SPEED -> MAX_SPEED -> AVG_SPEED -> TIME -> MOVING_TIME -> ELAPSED_TIME -> DISTANCE -> SPEED

  mode.next();
  EXPECT_EQ(mode.get(), ui::Mode::ID::MAX_SPEED);

  mode.next();
  EXPECT_EQ(mode.get(), ui::Mode::ID::AVG_SPEED);

  mode.next();
  EXPECT_EQ(mode.get(), ui::Mode::ID::TIME);

  mode.next();
  EXPECT_EQ(mode.get(), ui::Mode::ID::MOVING_TIME);

  mode.next();
  EXPECT_EQ(mode.get(), ui::Mode::ID::ELAPSED_TIME);

  mode.next();
  EXPECT_EQ(mode.get(), ui::Mode::ID::DISTANCE);

  mode.next();
  EXPECT_EQ(mode.get(), ui::Mode::ID::SPEED);
}
