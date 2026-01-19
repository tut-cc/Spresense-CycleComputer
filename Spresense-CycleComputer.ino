#include <LowPower.h>

#include "src2/App.h"

App app;

void setup() {
  LowPower.begin();
  LowPower.clockMode(CLOCK_MODE_32MHz);
  app.begin();
}

void loop() {
  app.update();
}
