/*
 * ファイル: SevenSegDriver.h
 * 説明: 7セグメントディスプレイ用のディスプレイドライバー（現在はシリアルを使用したモック実装）。
 */

#ifndef SEVENSEGDRIVER_H
#define SEVENSEGDRIVER_H

#include <Arduino.h>

#include "../interfaces/IDisplay.h"

//速度が代入される
int intspeed=0;

//各値が代入され、こいつが引数となる。
int dig1=0;
int dig2=0;
int dig3=0;
int dig4=0;


/*
class SevenSegDriver : public IDisplay {
   public:
    void begin() override {
        Serial.begin(115200);
        Serial.println("SevenSegDriver initialized");
    }

    void clear() override {
        Serial.println("[7SEG] CLEAR");
    }

    void show(DisplayDataType type, const char* value) override {
        Serial.print("[7SEG] ");
        Serial.println(value);
    }
};*/

class SevenSegDriver : public IDisplay {
   public:
    void begin() override {
        Serial.begin(115200);
        Serial.println("SevenSegDriver initialized");
    }

    void clear() override {
        Serial.println("[7SEG] CLEAR");
    }

    void show(DisplayDataType type, const char* value) override {
        Serial.print("[7SEG] ");
        Serial.println(value);
    }
};

#endif
