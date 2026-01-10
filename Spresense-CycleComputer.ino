#include "src/drivers/OLEDDriver.h"
#include "src/system/CycleComputer.h"

drivers::OLEDDriver display(Wire);
application::CycleComputer computer(&display);

void setup() {
  Serial.begin(115200);
  computer.begin();
}

void loop() {
  computer.update();
}
