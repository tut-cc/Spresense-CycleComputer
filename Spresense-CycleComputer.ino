#include <LowPower.h>

#include "src/App.h"

App app;

void setup() {
  LowPower.begin();
  LowPower.clockMode(CLOCK_MODE_32MHz);
  app.begin();
}

void loop() {
  app.update();
}
