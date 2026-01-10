#include "src/system/CycleComputer.h"
#include "src/drivers/OLEDDriver.h"

OLEDDriver display;
CycleComputer computer(&display);

void setup() {
    computer.begin();
}

void loop() {
    computer.update();
}
