/*
 * ファイル: CycleComputer.h
 * 説明: CycleComputerクラス
 */

#ifndef CYCLECOMPUTER_H
#define CYCLECOMPUTER_H

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
#if DISPLAY_TYPE == DISPLAY_I2C_LCD
  static const unsigned long DISPLAY_UPDATE_INTERVAL_MS = 1000; // 1 FPS for I2C
#else
  static const unsigned long DISPLAY_UPDATE_INTERVAL_MS =
      200; // 5 FPS for others
#endif

  void handleInput();
  void updateDisplay();

public:
  CycleComputer(IDisplay *display);
  void begin();
  void update();
};

#endif
