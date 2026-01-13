#include "ui/Mode.h"
#include <gtest/gtest.h>

class ModeTest : public ::testing::Test {
protected:
  Mode mode; // Renamed from modeManager
};

TEST_F(ModeTest, InitialModeIsSpeed) {
  EXPECT_EQ(mode.get(), Mode::ID::SPEED);
}

TEST_F(ModeTest, CycleThroughModes) {
  mode.next();
  EXPECT_EQ(mode.get(), Mode::ID::MAX_SPEED);

  mode.next();
  EXPECT_EQ(mode.get(), Mode::ID::AVG_SPEED);

  mode.next();
  EXPECT_EQ(mode.get(), Mode::ID::TIME);

  mode.next();
  EXPECT_EQ(mode.get(), Mode::ID::MOVING_TIME);

  mode.next();
  EXPECT_EQ(mode.get(), Mode::ID::ELAPSED_TIME);

  mode.next();
  EXPECT_EQ(mode.get(), Mode::ID::DISTANCE);

  mode.next();
  EXPECT_EQ(mode.get(), Mode::ID::SPEED);
}
