/*
 * ファイル: Spresense-CycleComputer.ino
 * 説明: サイクルコンピューターのエントリーポイント。
 */

#include "src/Config.h"

#ifdef ENABLE_POWER_SAVING
#include <LowPower.h>
#endif

#include "src/drivers/SevenSegDriver.h"
#include "src/drivers/OLEDDriver.h"
#include "src/system/CycleComputer.h"

#if DISPLAY_TYPE == DISPLAY_SEVENSEG
SevenSegDriver display;
#elif DISPLAY_TYPE == DISPLAY_OLED
OLEDDriver display;
#endif

CycleComputer computer(&display);

void setup() {
#ifdef ENABLE_POWER_SAVING
    LowPower.begin();
    LowPower.clockMode(CLOCK_MODE_8MHz);
#endif
    computer.begin();
}

void loop() {
    computer.update();
}
