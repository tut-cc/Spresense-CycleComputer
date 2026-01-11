#pragma once
#include "Config.h"

#include "domain/Clock.h"
#include "domain/Trip.h"
#include "hardware/Gnss.h"
#include "hardware/OLED.h"
#include "ui/Input.h"
#include "ui/InputEvent.h"
#include "ui/Mode.h"
#include "ui/Renderer.h"
#include <cstdio>

class App {
private:
  OLED          &display;
  Input         &input;
  Gnss          &gnss;
  Mode           mode;
  Trip           trip;
  Clock          clock;
  Renderer<OLED> renderer;

public:
  App(OLED &displayData, Gnss &gnss, Input &inputModule);

  void begin();
  void update();

private:
  void handleInput();
};
