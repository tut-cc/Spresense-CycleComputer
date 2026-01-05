/*
 * ファイル: CycleComputer.cpp
 * 説明: 入力、GPSデータ、およびディスプレイ更新を調整するメインロジック。
 */

#include "CycleComputer.h"
#include "../Config.h"

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
    if (!forceUpdate && (currentMillis - lastDisplayUpdate < Config::DISPLAY_UPDATE_INTERVAL_MS)) return;

    lastDisplayUpdate = currentMillis;
    forceUpdate = false;

    // 表示データの取得
    char buf[20];
    DisplayDataType type;
    getDisplayData(modeManager.getMode(), type, buf, sizeof(buf));

#ifdef DEBUGDAO
    // 前回のモードを記憶しておく変数 (staticなので値を保持し続けます)
    static int lastDebugMode = -1;
    
    // 現在のモードを取得して比較
    int currentDebugMode = (int)modeManager.getMode();
    if (lastDebugMode != currentDebugMode) {
        Serial.print("[CycleComputer] Mode changed: ");
        Serial.print(currentDebugMode);
        Serial.print(" Value: ");
        Serial.println(buf);
        lastDebugMode = currentDebugMode;
    }
#endif

    // ディスプレイ更新
    display->show(type, buf);
}

void CycleComputer::getDisplayData(Mode mode, DisplayDataType& type, char* buf, size_t size) {
    switch (mode) {
        case Mode::SPEED:
            type = DisplayDataType::SPEED;
            formatFloat(gps.getSpeedKmh(), 4, 1, buf, size);
            break;
        case Mode::TIME:
            type = DisplayDataType::TIME;
            gps.getTimeJST(buf, size);
            break;
        case Mode::MAX_SPEED:
            type = DisplayDataType::MAX_SPEED;
            formatFloat(tripComputer.getMaxSpeedKmh(), 4, 1, buf, size);
            break;
        case Mode::DISTANCE:
            type = DisplayDataType::DISTANCE;
            formatFloat(tripComputer.getDistanceKm(), 5, 2, buf, size);
            break;
        case Mode::MOVING_TIME:
            type = DisplayDataType::MOVING_TIME;
            tripComputer.getMovingTimeStr(buf, size);
            break;
        case Mode::ELAPSED_TIME:
            type = DisplayDataType::ELAPSED_TIME;
            tripComputer.getElapsedTimeStr(buf, size);
            break;
        case Mode::AVG_SPEED:
            type = DisplayDataType::AVG_SPEED;
            formatFloat(tripComputer.getAvgSpeedKmh(), 4, 1, buf, size);
            break;
        default:
            type = DisplayDataType::INVALID; // Assuming DISPLAY_INVALID exists or use a default
            buf[0] = '\0';
            break;
    }
}

void CycleComputer::checkBattery() {
#ifdef ENABLE_POWER_SAVING
    unsigned long currentMillis = millis();

    // 10秒ごとに電圧をチェック
    if (currentMillis - lastBatteryCheck >= Config::Power::BatteryCheckIntervalMs) {
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
    if (isLowBattery) {
        // ブロッキングなしの単純な点滅のために時間の剰余を使用
        if ((currentMillis / Config::Power::LedBlinkIntervalMs) % 2 == 0)   digitalWrite(Config::Pin::WarnLed, HIGH);
        else                                                                digitalWrite(Config::Pin::WarnLed, LOW);
    }
#endif
}
