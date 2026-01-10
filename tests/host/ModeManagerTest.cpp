#include <gtest/gtest.h>

#include "system/ModeManager.h"

class ModeManagerTest : public ::testing::Test {
    // No special setup needed for ModeManager currently
};

TEST_F(ModeManagerTest, InitialModeIsSpeed) {
    ModeManager mm;
    EXPECT_EQ(mm.getMode(), Mode::SPEED);
}

TEST_F(ModeManagerTest, CycleThroughModes) {
    ModeManager mm;

    // SPEED -> TIME
    mm.nextMode();
    EXPECT_EQ(mm.getMode(), Mode::TIME);

    // TIME -> MAX_SPEED
    mm.nextMode();
    EXPECT_EQ(mm.getMode(), Mode::MAX_SPEED);

    // MAX_SPEED -> DISTANCE
    mm.nextMode();
    EXPECT_EQ(mm.getMode(), Mode::DISTANCE);

    // DISTANCE -> MOVING_TIME
    mm.nextMode();
    EXPECT_EQ(mm.getMode(), Mode::MOVING_TIME);

    // MOVING_TIME -> ELAPSED_TIME
    mm.nextMode();
    EXPECT_EQ(mm.getMode(), Mode::ELAPSED_TIME);

    // ELAPSED_TIME -> AVG_SPEED
    mm.nextMode();
    EXPECT_EQ(mm.getMode(), Mode::AVG_SPEED);

    // AVG_SPEED -> SPEED (Wrap around)
    mm.nextMode();
    EXPECT_EQ(mm.getMode(), Mode::SPEED);
}
