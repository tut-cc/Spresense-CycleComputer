/*
 * ファイル: SevenSegDriver.h
 * 説明:
 * 7セグメントディスプレイ用のディスプレイドライバー。
 */

#pragma once

#include <Arduino.h>

#include "../Config.h"
#include "../interfaces/IDisplay.h"

// 速度が代入される
int molding = 0;

// 各値が代入され、こいつが引数となる。
int dig1 = 0;
int dig2 = 0;
int dig3 = 0;
int dig4 = 0;

const int placePin[] = {PIN_D10, PIN_D11, PIN_D12, PIN_D13};
// 7セグメントのデータ
// 0, 1, 2, ... 9,ハイフン(表示するまで待つ用のやつ)
// 左からabcdefg,dotに対応する
unsigned char number[] = {0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90, 0xbf};
unsigned char numdot[] = {0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02, 0x78, 0x00, 0x10, 0xbf};

// 桁を選択する関数
void pickDigit(int digit) {
    for (int i = 0; i < 4; i++) {
        digitalWrite(placePin[i], LOW);  // 全てOFF
    }
    digitalWrite(placePin[digit], HIGH);  // 指定の桁だけON
}

// シフトレジスタにデータを送る関数
// Arduino標準のshiftOutを使うとシンプルに書けます
void hc595_shift(byte data) {
    digitalWrite(RCLK_PIN, LOW);  // 送信準備

    // MSBFIRST: 最上位ビットから順に送信
    shiftOut(SDI_PIN, SRCLK_PIN, MSBFIRST, data);

    digitalWrite(RCLK_PIN, HIGH);  // ラッチ(出力反映)
}

// 表示をクリアする関数
void clearDisplay() {
    // 0xFF (すべて1) を送ると、Common Anodeの場合は全消灯
    hc595_shift(0xFF);
}

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
        // ピンモード設定
        pinMode(SDI_PIN, OUTPUT);
        pinMode(RCLK_PIN, OUTPUT);
        pinMode(SRCLK_PIN, OUTPUT);

        for (int i = 0; i < 4; i++) {
            pinMode(placePin[i], OUTPUT);
            digitalWrite(placePin[i], LOW);  // 初期状態はOFFにしておく(High ActiveならLOWでOFF)
        }

        // --- ダイナミック点灯処理 (高速切り替え) ---
        // atofを使って、文字列を一度「小数(float)」に変換する
        float f_val = atof(value);

        // その数値を100倍して整数に入れる
        molding = (int)(f_val * 100);
        dig1 = (molding / 1000);

#ifndef IS_SPRESENSE
        dig1 = 1.234;
#endif

        // 受け取った値が元の値が1000以上だったら
            if (dig1 != 0) {
            dig2 = (molding / 100) % 10;
            dig3 = (molding / 10) % 10;
            dig4 = molding % 10;
        } else {
            dig2 = (molding / 100);
            dig3 = (molding / 10) % 10;
            dig4 = molding % 10;
        }

        // Serial.print("[7SEG] ");
        // Serial.println(value);

        clearDisplay();
        pickDigit(0);
        hc595_shift(number[dig1]);
        delayMicroseconds(2000);  // ちらつき防止と明るさ確保のためのわずかな待機

        clearDisplay();
        pickDigit(1);
        hc595_shift(numdot[dig2]);
        delayMicroseconds(2000);

        clearDisplay();
        pickDigit(2);
        hc595_shift(number[dig3]);
        delayMicroseconds(2000);

        clearDisplay();
        pickDigit(3);
        hc595_shift(number[dig4]);
        delayMicroseconds(2000);
    }
};

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
