#pragma once

#include "../hal/interfaces/IDisplay.h"
#include "../hal/interfaces/IGnssProvider.h"
#include "../hal/interfaces/IInputProvider.h"
#include "ModeManager.h"
#include "PowerManager.h"
#include "TripManager.h"

namespace application {

class CycleComputer {
private:
  hal::IDisplay       *display;
  hal::IInputProvider &inputProvider;
  hal::IGnssProvider  &gnssProvider;
  ModeManager          modeManager;
  TripManager          tripManager;
  PowerManager         powerManager;

  unsigned long lastDisplayUpdate = 0;
  bool          forceUpdate       = false;

  void handleInput();
  void updateDisplay();
  void getDisplayData(Mode mode, DisplayDataType &type, char *buf, size_t size);

public:
  CycleComputer(hal::IDisplay *display, hal::IGnssProvider &gnss, hal::IInputProvider &input);
  void begin();
  void update();
};

} // namespace application
