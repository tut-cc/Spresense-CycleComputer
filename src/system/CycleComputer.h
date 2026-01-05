#pragma once

#include "../drivers/GPSWrapper.h"
#include "../interfaces/IDisplay.h"
#include "InputManager.h"
#include "ModeManager.h"
#include "TripComputer.h"
#include "PowerManager.h"

class CycleComputer {
   private:
    IDisplay* display;
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

   public:
    CycleComputer(IDisplay* display);
    void begin();
    void update();
};
