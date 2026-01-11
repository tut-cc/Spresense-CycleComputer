#include <LowPower.h>

#include "src/App.h"
#include "src/hardware/Button.h"
#include "src/hardware/Gnss.h"
#include "src/hardware/OLED.h"
#include "src/ui/Input.h"

OLED   oled;
Gnss   gnss;
Button btnA(Config::Pin::BTN_A);
Button btnB(Config::Pin::BTN_B);
Input  input(btnA, btnB);
App    app(oled, gnss, input);

void setup() {
  LowPower.begin();
  LowPower.clockMode(CLOCK_MODE_32MHz);
  app.begin();
}

void loop() {
  app.update();
}
