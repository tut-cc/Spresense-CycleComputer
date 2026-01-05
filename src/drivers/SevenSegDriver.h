/*
 * ファイル: SevenSegDriver.h
 * 説明:
 * 7セグメントディスプレイ用のディスプレイドライバー。
 */

#pragma once

#include <Arduino.h>

#include "../interfaces/IDisplay.h"

class SevenSegDriver : public IDisplay {
   public:
    void begin() override;
    void clear() override;
    void show(DisplayDataType type, const char* value) override;

   private:
    // ヘルパー関数
    void pickDigit(int digit);
    void hc595_shift(byte data);
    void clearDisplay();

    // 内部状態変数
    int molding = 0;
    int dig1 = 0;
    int dig2 = 0;
    int dig3 = 0;
    int dig4 = 0;

    // 定数データ
    static const int placePin[4];
    static const int colonPin[5];
    static const unsigned char* number;
    static const unsigned char* numdot;
};
