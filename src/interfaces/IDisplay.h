/*
 * ファイル: IDisplay.h
 * 説明: ディスプレイドライバーのインターフェース
 */

#pragma once

#include <Arduino.h>

enum DisplayDataType {
    DISPLAY_DATA_SPEED,
    DISPLAY_DATA_TIME,
    DISPLAY_DATA_MAX_SPEED,
    DISPLAY_DATA_DISTANCE,
    DISPLAY_DATA_MOVING_TIME,
    DISPLAY_DATA_ELAPSED_TIME,
    DISPLAY_DATA_AVG_SPEED
};

class IDisplay {
   public:
    virtual ~IDisplay() {}
    virtual void begin() = 0;
    virtual void clear() = 0;
    virtual void show(DisplayDataType type, const char* value) = 0;
};
