/*
 * ファイル: CycleComputer.h
 * 説明: CycleComputerクラス
 */

#pragma once

#include "../drivers/GPSWrapper.h"
#include "../interfaces/IDisplay.h"
#include "InputManager.h"
#include "ModeManager.h"
#include "TripComputer.h"

class CycleComputer {
   private:
    IDisplay *display;
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
    static const unsigned long DISPLAY_UPDATE_INTERVAL_MS = 0;  // 5 FPS for others

    void handleInput();
    void updateDisplay();

   public:
    CycleComputer(IDisplay *display);
    void begin();
    void update();
};
