#pragma once

#include "../drivers/GPSWrapper.h"
#include "../drivers/OLEDDriver.h"
#include "InputManager.h"
#include "ModeManager.h"
#include "TripComputer.h"
#include "PowerManager.h"

class CycleComputer {
   private:
    IDisplay* display;
    bool ownsDisplay = false; // Flag to check if we need to delete display
    InputManager inputManager;
    ModeManager modeManager;
    GPSWrapper gps;
    TripComputer tripComputer;
    PowerManager powerManager;

    // ディスプレイ更新制御
    unsigned long lastDisplayUpdate = 0;
    bool forceUpdate = false;

    void handleInput();
    void updateDisplay();
    void getDisplayData(Mode mode, DisplayDataType& type, char* buf, size_t size);
    void logDebugInfo(Mode currentMode, const char* value);

   public:
    CycleComputer();
    CycleComputer(IDisplay* display);
    ~CycleComputer();
    void begin();
    void update();
};
