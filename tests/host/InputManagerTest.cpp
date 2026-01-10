#include <gtest/gtest.h>

#include "Arduino.h"
#include "Config.h"
#include "system/InputManager.h"

class InputManagerTest : public ::testing::Test {
   protected:
    void SetUp() override {
        _mock_millis = 1000;
        _mock_pin_states.clear();
        // Init default logic state (PULLUP -> HIGH)
        setPinState(Config::Pin::BTN_A, HIGH);
        setPinState(Config::Pin::BTN_B, HIGH);
    }
};

TEST_F(InputManagerTest, InitialStateReturnsNone) {
    InputManager input;
    input.begin();

    EXPECT_EQ(input.update(), InputEvent::NONE);
}

TEST_F(InputManagerTest, ButtonAPress) {
    InputManager input;
    input.begin();

    // Not pressed yet
    EXPECT_EQ(input.update(), InputEvent::NONE);

    // Press A (LOW)
    setPinState(Config::Pin::BTN_A, LOW);

    // Simulate debounce logic
    // Button.cpp requires state change then timeout
    // 1. Detect change
    input.update();

    // 2. Wait debounce time (50ms)
    delay(51);

    // 3. Update again to confirm state
    InputEvent evt = input.update();

    EXPECT_EQ(evt, InputEvent::BTN_A);
}

TEST_F(InputManagerTest, ButtonBPress) {
    InputManager input;
    input.begin();

    setPinState(Config::Pin::BTN_B, LOW);
    input.update();
    delay(51);

    EXPECT_EQ(input.update(), InputEvent::BTN_B);
}

TEST_F(InputManagerTest, SimultaneousPress) {
    InputManager input;
    input.begin();

    // To simulate simultaneous press correctly with the current logic:
    // (aPressed && btnB.isHeld()) || (bPressed && btnA.isHeld())

    // Let's press both
    setPinState(Config::Pin::BTN_A, LOW);
    setPinState(Config::Pin::BTN_B, LOW);

    input.update();
    delay(51);

    // Both are held now
    EXPECT_EQ(input.update(), InputEvent::BTN_BOTH);
}

TEST_F(InputManagerTest, ButtonRelease) {
    InputManager input;
    input.begin();

    // Press A
    setPinState(Config::Pin::BTN_A, LOW);
    input.update();
    delay(51);
    input.update();  // Returns BTN_A

    // Release A
    setPinState(Config::Pin::BTN_A, HIGH);
    input.update();
    delay(51);

    EXPECT_EQ(input.update(), InputEvent::NONE);
}
