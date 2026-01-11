#include "src/drivers/OLEDDriver.h"
#include "src/system/CycleComputer.h"
#include "src/system/GPSManager.h"
#include "src/system/InputManager.h"

drivers::OLEDDriver        display(Wire);
application::GPSManager    gps;
application::InputManager  inputManager;
application::CycleComputer computer(&display, gps, inputManager);

void setup() {
  computer.begin();
}

void loop() {
  computer.update();
}
