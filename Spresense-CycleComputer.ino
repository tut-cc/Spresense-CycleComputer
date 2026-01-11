#include <LowPower.h>

#include "src/CycleComputer.h"
#include "src/drivers/Gnss.h"
#include "src/drivers/OLED.h"
#include "src/system/Input.h"

drivers::OLED                                                                display(Wire);
application::Input                                                           input;
application::CycleComputer<drivers::OLED, drivers::Gnss, application::Input> computer(display, drivers::Gnss::getInstance(), input);

void setup() {
  LowPower.begin();
  LowPower.clockMode(CLOCK_MODE_32MHz);
  computer.begin();
}

void loop() {
  computer.update();
}
