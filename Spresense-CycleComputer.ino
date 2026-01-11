#include <LowPower.h>

#include "src/CycleComputer.h"
#include "src/drivers/Button.h"
#include "src/drivers/Gnss.h"
#include "src/drivers/OLED.h"

using namespace drivers;

OLED                                           oled;
Button                                         btnA(Config::Pin::BTN_A);
Button                                         btnB(Config::Pin::BTN_B);
application::CycleComputer<OLED, Gnss, Button> computer(oled, Gnss::getInstance(), btnA, btnB);

void setup() {
  LowPower.begin();
  LowPower.clockMode(CLOCK_MODE_32MHz);
  computer.begin();
}

void loop() {
  computer.update();
}
