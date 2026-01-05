#include "src/Config.h"
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
    computer.begin();
}

void loop() {
    computer.update();
}
