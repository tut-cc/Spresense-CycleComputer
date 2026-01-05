#include "src/Config.h"

#ifdef ENABLE_POWER_SAVING
#include <LowPower.h>
#endif

#include "src/system/CycleComputer.h"

#if DISPLAY_TYPE == DISPLAY_SEVENSEG
#include "src/drivers/SevenSegDriver.h"
SevenSegDriver display;
#elif DISPLAY_TYPE == DISPLAY_OLED
#include "src/drivers/OLEDDriver.h"
OLEDDriver display;
#endif

CycleComputer computer(&display);

void setup() {
#ifdef ENABLE_POWER_SAVING
    LowPower.begin();
    LowPower.clockMode(CLOCK_MODE_8MHz);
#endif
    computer.begin();
}

void loop() {
    computer.update();
}
