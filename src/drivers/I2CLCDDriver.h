/*
 * ファイル: I2CLCDDriver.h
 * 説明: I2C接続のLCD用ディスプレイドライバー。
 */

#pragma once

#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#include "../interfaces/IDisplay.h"

class I2CLCDDriver : public IDisplay {
   private:
    LiquidCrystal_I2C lcd;
    char shadowLine1[17];
    char shadowLine2[17];

    // ヘルパー: 行更新
    void updateLine(int row, char *shadow, const char *newContent);

   public:
    I2CLCDDriver();

    void begin() override;

    void clear() override;

    void show(DisplayDataType type, const char *value) override;
};
