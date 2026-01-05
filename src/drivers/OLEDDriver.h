/*
 * ファイル: OLEDDriver.h
 * 説明:
 * OLEDディスプレイ用のディスプレイドライバー。
 */

#pragma once

#include <Arduino.h>

#include "../Config.h"
#include "../interfaces/IDisplay.h"

class OLEDDriver : public IDisplay {
   public:
    void begin() override;
    void clear() override;
    void show(DisplayDataType type, const char* value) override;
};
