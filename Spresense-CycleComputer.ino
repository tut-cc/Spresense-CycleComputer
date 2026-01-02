/*
 * ファイル: Spresense-CycleComputer.ino
 * 説明: サイクルコンピューターのエントリーポイント。
 *       設定に基づいてドライバーとシステムコンポーネントを初期化します。
 */

#include "src/Config.h"

#ifdef ENABLE_POWER_SAVING
#include <LowPower.h>
#endif

#include "src/drivers/I2CLCDDriver.h"
#include "src/drivers/LCDDriver.h"
#include "src/drivers/SevenSegDriver.h"
#include "src/system/CycleComputer.h"

#if DISPLAY_TYPE == DISPLAY_LCD
LCDDriver display;
#elif DISPLAY_TYPE == DISPLAY_I2C_LCD
I2CLCDDriver display;
#elif DISPLAY_TYPE == DISPLAY_SEVENSEG
SevenSegDriver display;
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
    // 入力に対する応答性を最大化するため、ブロッキング遅延はなし
}
