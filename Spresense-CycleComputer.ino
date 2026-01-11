#include <LowPower.h>

#include "src/App.h"
#include "src/hardware/Button.h"
#include "src/hardware/Gnss.h"
#include "src/hardware/OLED.h"
#include "src/ui/Input.h"

using namespace hardware;

OLED                                            oled;
Button                                          btnA(Config::Pin::BTN_A);
Button                                          btnB(Config::Pin::BTN_B);
ui::Input<Button>                               input(btnA, btnB);
application::App<OLED, Gnss, ui::Input<Button>> app(oled, Gnss::getInstance(), input);

void setup() {
  LowPower.begin();
  LowPower.clockMode(CLOCK_MODE_32MHz);
  app.begin();
}

void loop() {
  app.update();
}
