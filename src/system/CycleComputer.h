#pragma once

#include "../drivers/GPSWrapper.h"
#include "../drivers/OLEDDriver.h"
#include "InputManager.h"
#include "ModeManager.h"
#include "PowerManager.h"
#include "TripComputer.h"

class CycleComputer {
   private:
    OLEDDriver* display;
    InputManager inputManager;
    ModeManager modeManager;
    GPSWrapper gps;
    TripComputer tripComputer;
    PowerManager powerManager;

    unsigned long lastDisplayUpdate = 0;
    bool forceUpdate = false;

    void handleInput();
    void updateDisplay();
    void getDisplayData(Mode mode, DisplayDataType& type, char* buf, size_t size);

   public:
    CycleComputer(OLEDDriver* display);
    void begin();
    void update();
};
