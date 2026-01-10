#include <gtest/gtest.h>

#include "system/ModeManager.h"

class ModeManagerTest : public ::testing::Test {
  // No special setup needed for ModeManager currently
};

TEST_F(ModeManagerTest, InitialModeIsSpeed) {
  application::ModeManager mm;
  EXPECT_EQ(mm.getMode(), application::Mode::SPEED);
}

TEST_F(ModeManagerTest, CycleThroughModes) {
  application::ModeManager mm;

  // SPEED -> MAX_SPEED
  mm.nextMode();
  EXPECT_EQ(mm.getMode(), application::Mode::MAX_SPEED);

  // MAX_SPEED -> AVG_SPEED
  mm.nextMode();
  EXPECT_EQ(mm.getMode(), application::Mode::AVG_SPEED);

  // AVG_SPEED -> TIME
  mm.nextMode();
  EXPECT_EQ(mm.getMode(), application::Mode::TIME);

  // TIME -> MOVING_TIME
  mm.nextMode();
  EXPECT_EQ(mm.getMode(), application::Mode::MOVING_TIME);

  // MOVING_TIME -> ELAPSED_TIME
  mm.nextMode();
  EXPECT_EQ(mm.getMode(), application::Mode::ELAPSED_TIME);

  // ELAPSED_TIME -> DISTANCE
  mm.nextMode();
  EXPECT_EQ(mm.getMode(), application::Mode::DISTANCE);

  // DISTANCE -> SPEED
  mm.nextMode();
  EXPECT_EQ(mm.getMode(), application::Mode::SPEED);
}
// I will rewrite the test content assuming the Step 174 Enum Order.
