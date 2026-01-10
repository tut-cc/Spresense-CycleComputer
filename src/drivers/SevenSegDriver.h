#pragma once

#include <Arduino.h>

#include "../drivers/OLEDDriver.h"

// 7セグメントシフトレジスタ
constexpr int SDI = PIN_D09;
constexpr int RCLK = PIN_D08;
constexpr int SRCLK = PIN_D07;

// 7セグメント桁指定 (Common Anode/Cathode selection pins)
constexpr int D1 = PIN_D10;
constexpr int D2 = PIN_D11;
constexpr int D3 = PIN_D12;
constexpr int D4 = PIN_D13;

// 7セグメントコロン/ドット制御
constexpr int D1_COLON = PIN_D03;
constexpr int D2_COLON = PIN_D04;
constexpr int D3_COLON = PIN_D05;
constexpr int D4_COLON = PIN_D06;
constexpr int DOT_COLON = PIN_D07;

// アノードコモンにおける数字表示用16進数
// 0, 1, 2, ... 9, ハイフン
constexpr unsigned char NUMBERS[] = {0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92,
                                     0x82, 0xf8, 0x80, 0x90, 0xbf};

// ドット付表示用
constexpr unsigned char NUMBERS_WITH_DOT[] = {0x40, 0x79, 0x24, 0x30, 0x19, 0x12,
                                              0x02, 0x78, 0x00, 0x10, 0xbf};

class SevenSegDriver {
   public:
    void begin();
    void clear();
    void show(DisplayDataType type, const char* value);
    void update();

   private:
    void hc595_shift(byte data);

    // Internal state
    byte digitBuffer[4] = {0, 0, 0, 0};  // Buffer for 4 digits (segment data)
    int currentDigit = 0;                // Currently active digit (0-3) for multiplexing
    unsigned long lastMultiplexTime = 0;
    const unsigned long MULTIPLEX_INTERVAL_US = 2000;  // 2ms per digit

    // Constant data
    static const int placePin[4];
    static const int colonPin[5];
    static const unsigned char* number;
    static const unsigned char* numdot;
};
