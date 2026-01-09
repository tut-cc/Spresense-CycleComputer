#include "src/system/CycleComputer.h"

CycleComputer computer;

void setup() {
    computer.begin();
}

void loop() {
    computer.update();
}
