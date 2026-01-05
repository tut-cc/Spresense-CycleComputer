#pragma once

#include "../drivers/GPSWrapper.h"
#include "../interfaces/IDisplay.h"
#include "InputManager.h"
#include "ModeManager.h"
#include "TripComputer.h"

class CycleComputer {
   private:
    IDisplay* display;
    InputManager inputManager;
    ModeManager modeManager;
    GPSWrapper gps;
    TripComputer tripComputer;

    // 電源電圧監視
    unsigned long lastBatteryCheck = 0;
    bool isLowBattery = false;
    void checkBattery();

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
