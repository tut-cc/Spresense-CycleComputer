/*
 * ファイル: CycleComputer.cpp
 * 説明: 入力、GPSデータ、およびディスプレイ更新を調整するメインロジック。
 */

#include "CycleComputer.h"

#include "Utils.h"

#ifdef ENABLE_POWER_SAVING
#include <LowPower.h>
#endif

CycleComputer::CycleComputer(IDisplay *display) : display(display) {}

void CycleComputer::begin() {
    display->begin();
    inputManager.begin();
    gps.begin();
    tripComputer.begin();

#ifdef ENABLE_POWER_SAVING
    pinMode(WARN_LED, OUTPUT);
    digitalWrite(WARN_LED, LOW);
#endif
}

void CycleComputer::update() {
    handleInput();

    gps.update();
    tripComputer.update(gps.getSpeedKmh(), millis());

    checkBattery();

    updateDisplay();
}

void CycleComputer::handleInput() {
    InputEvent event = inputManager.update();

    switch (event) {
        case INPUT_BTN_A:
            modeManager.nextMode();
// 画面が乱れることがあるため、I2C LCDの場合のみ再初期化して復帰させる
#if DISPLAY_TYPE == DISPLAY_I2C_LCD
            display->begin();
#endif
            forceUpdate = true;
            break;
        case INPUT_BTN_BOTH:
            tripComputer.reset();
#if DISPLAY_TYPE == DISPLAY_I2C_LCD
            display->begin();
#endif
            forceUpdate = true;
            break;
        case INPUT_BTN_B:
            // 将来のプレースホルダー
            break;
        default:
            break;
    }
}

void CycleComputer::updateDisplay() {
    unsigned long currentMillis = millis();

    // 強制更新フラグがなく、かつ更新間隔未満であればスキップ
    if (!forceUpdate && (currentMillis - lastDisplayUpdate < DISPLAY_UPDATE_INTERVAL_MS)) {
        return;
    }

    lastDisplayUpdate = currentMillis;
    forceUpdate = false;

    char buf[20];

    switch (modeManager.getMode()) {
        case MODE_SPEED:
            formatFloat(gps.getSpeedKmh(), 4, 1, buf, sizeof(buf));
            display->show(DISPLAY_DATA_SPEED, buf);
            break;
        case MODE_TIME:
            gps.getTimeJST(buf, sizeof(buf));
            display->show(DISPLAY_DATA_TIME, buf);
            break;
        case MODE_MAX_SPEED:
            formatFloat(tripComputer.getMaxSpeedKmh(), 4, 1, buf, sizeof(buf));
            display->show(DISPLAY_DATA_MAX_SPEED, buf);
            break;
        case MODE_DISTANCE:
            formatFloat(tripComputer.getDistanceKm(), 5, 2, buf, sizeof(buf));
            display->show(DISPLAY_DATA_DISTANCE, buf);
            break;
        case MODE_MOVING_TIME:
            tripComputer.getMovingTimeStr(buf, sizeof(buf));
            display->show(DISPLAY_DATA_MOVING_TIME, buf);
            break;
        case MODE_ELAPSED_TIME:
            tripComputer.getElapsedTimeStr(buf, sizeof(buf));
            display->show(DISPLAY_DATA_ELAPSED_TIME, buf);
            break;
        case MODE_AVG_SPEED:
            formatFloat(tripComputer.getAvgSpeedKmh(), 4, 1, buf, sizeof(buf));
            display->show(DISPLAY_DATA_AVG_SPEED, buf);
            break;
    }
}

void CycleComputer::checkBattery() {
#ifdef ENABLE_POWER_SAVING
    unsigned long currentMillis = millis();

    // 10秒ごとに電圧をチェック
    if (currentMillis - lastBatteryCheck >= BATTERY_CHECK_INTERVAL_MS) {
        lastBatteryCheck = currentMillis;
        int voltage = LowPower.getVoltage();
        // 電圧が有効であり（USBでは0を返すことがある）、かつ閾値を下回っているかを確認
        if (voltage > 0 && voltage < BATTERY_LOW_THRESHOLD) {
            isLowBattery = true;
        } else {
            isLowBattery = false;
            digitalWrite(WARN_LED, LOW);  // 安全な場合はLEDがオフであることを保証する
        }
    }

    // バッテリー残量が少ない場合、LEDを点滅させる
    // 点滅パターン: 500ms 点灯 / 500ms 消灯
    if (isLowBattery) {
        // ブロッキングなしの単純な点滅ケイデンスのために時間の剰余を使用
        // (currentMillis / 500) % 2 == 0 は、0-499ms: 点灯、500-999ms:
        // 消灯を意味する
        if ((currentMillis / LED_BLINK_INTERVAL_MS) % 2 == 0) {
            digitalWrite(WARN_LED, HIGH);
        } else {
            digitalWrite(WARN_LED, LOW);
        }
    }
#endif
}
