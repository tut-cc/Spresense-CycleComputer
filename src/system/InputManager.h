#pragma once

#include "../drivers/Button.h"

enum InputEvent {
    INPUT_BTN_A = PIN_D00,
    INPUT_BTN_B = PIN_D01,
    INPUT_BTN_BOTH,
    INPUT_NONE,
};

class InputManager {
   private:
    Button btnA;
    Button btnB;

   public:
    InputManager();

    void begin();

    InputEvent update();
};
