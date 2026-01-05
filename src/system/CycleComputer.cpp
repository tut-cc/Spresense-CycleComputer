/*
 * ファイル: CycleComputer.cpp
 * 説明: 入力、GPSデータ、およびディスプレイ更新を調整するメインロジック。
 */

#include "CycleComputer.h"

#include "Utils.h"

#ifdef ENABLE_POWER_SAVING
#include <LowPower.h>
#endif

CycleComputer::CycleComputer(IDisplay* display) : display(display) {}

void CycleComputer::begin() {
    display->begin();
    inputManager.begin();
    gps.begin();
    tripComputer.begin();

#ifdef ENABLE_POWER_SAVING
    pinMode(Config::Pin::WarnLed, OUTPUT);
    digitalWrite(Config::Pin::WarnLed, LOW);
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
            forceUpdate = true;
            break;
        case INPUT_BTN_BOTH:
            tripComputer.reset();
            forceUpdate = true;
            break;
        case INPUT_BTN_B:
            break;
        default:
            break;
    }
}

void CycleComputer::updateDisplay() {
    unsigned long currentMillis = millis();

    // 強制更新フラグがなく、かつ更新間隔未満であればスキップ
    if (!forceUpdate && (currentMillis - lastDisplayUpdate < DISPLAY_UPDATE_INTERVAL_MS)) return;

    lastDisplayUpdate = currentMillis;
    forceUpdate = false;

    // デバッグ用の処理
    #ifdef DEBUGDAO
    // 前回のモードを記憶しておく変数 (staticなので値を保持し続けます)
    static int lastDebugMode = -1;
    
    // 今回表示すべきかどうかのフラグ
    bool shouldPrintDebug = false;
    
    // 現在のモードを取得して比較
    int currentDebugMode = (int)modeManager.getMode();
    if (lastDebugMode != currentDebugMode) {
        shouldPrintDebug = true;     // モードが変わったので表示許可
        lastDebugMode = currentDebugMode; // 今回のモードを記憶
    }
    #endif

    char buf[20];

    switch (modeManager.getMode()) {
        case MODE_SPEED:
            formatFloat(gps.getSpeedKmh(), 4, 1, buf, sizeof(buf));
#ifdef DEBUGDAO
            // ▼ モード切替時のみ表示
            if (shouldPrintDebug) {
                Serial.print("[CycleComputer] モード：速度");
                Serial.println(buf);
            }
#endif
            display->show(DISPLAY_DATA_SPEED, buf);
            break;
        case MODE_TIME:
            gps.getTimeJST(buf, sizeof(buf));
#ifdef DEBUGDAO
            if (shouldPrintDebug) {
                Serial.print("[CycleComputer] モード：TimeJST");
                Serial.println(buf);
            }
#endif
            display->show(DISPLAY_DATA_TIME, buf);
            break;
        case MODE_MAX_SPEED:
            formatFloat(tripComputer.getMaxSpeedKmh(), 4, 1, buf, sizeof(buf));
#ifdef DEBUGDAO
            if (shouldPrintDebug) {
                Serial.print("[CycleComputer] モード：MAXSPEED");
                Serial.println(buf);
            }
#endif
            display->show(DISPLAY_DATA_MAX_SPEED, buf);
            break;
        case MODE_DISTANCE:
            formatFloat(tripComputer.getDistanceKm(), 5, 2, buf, sizeof(buf));
#ifdef DEBUGDAO
            if (shouldPrintDebug) {
                Serial.print("[CycleComputer] モード：走行距離");
                Serial.println(buf);
            }
#endif
            display->show(DISPLAY_DATA_DISTANCE, buf);
            break;
        case MODE_MOVING_TIME:
            tripComputer.getMovingTimeStr(buf, sizeof(buf));
#ifdef DEBUGDAO
            if (shouldPrintDebug) {
                Serial.print("[CycleComputer] モード：走行時間");
                Serial.println(buf);
            }
#endif
            display->show(DISPLAY_DATA_MOVING_TIME, buf);
            break;
        case MODE_ELAPSED_TIME:
            tripComputer.getElapsedTimeStr(buf, sizeof(buf));
#ifdef DEBUGDAO
            if (shouldPrintDebug) {
                Serial.print("[CycleComputer] モード：経過時間");
                Serial.println(buf);
            }
#endif
            display->show(DISPLAY_DATA_ELAPSED_TIME, buf);
            break;
        case MODE_AVG_SPEED:
            formatFloat(tripComputer.getAvgSpeedKmh(), 4, 1, buf, sizeof(buf));
#ifdef DEBUGDAO
            if (shouldPrintDebug) {
                Serial.print("[CycleComputer] モード：平均速度");
                Serial.println(buf);
            }
#endif
            display->show(DISPLAY_DATA_AVG_SPEED, buf);
            break;
    }
}

void CycleComputer::checkBattery() {
    /*
#ifdef ENABLE_POWER_SAVING
    unsigned long currentMillis = millis();

    // 10秒ごとに電圧をチェック
    if (currentMillis - lastBatteryCheck >= BATTERY_CHECK_INTERVAL_MS) {
        lastBatteryCheck = currentMillis;
        int voltage = LowPower.getVoltage();
        // 電圧が有効であり（USBでは0を返すことがある）、かつ閾値を下回っているかを確認
            if (voltage > 0 && voltage < Config::Power::BatteryLowThreshold) {
                isLowBattery = true;
            } else {
                isLowBattery = false;
                digitalWrite(Config::Pin::WarnLed, LOW);  // 安全な場合はLEDがオフであることを保証する
            }
        }

        // バッテリー残量が少ない場合、LEDを点滅させる
        // 点滅パターン: 500ms 点灯 / 500ms 消灯
        if (isLowBattery) {
            // ブロッキングなしの単純な点滅ケイデンスのために時間の剰余を使用
            // (currentMillis / 500) % 2 == 0 は、0-499ms: 点灯、500-999ms:
            // 消灯を意味する
            if ((currentMillis / Config::Power::LedBlinkIntervalMs) % 2 == 0) {
                digitalWrite(Config::Pin::WarnLed, HIGH);
            } else {
                digitalWrite(Config::Pin::WarnLed, LOW);
            }
        }
#endif
    */
}
