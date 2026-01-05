/*
 * ファイル: SevenSegDriver.cpp
 * 説明: SevenSegDriverの実装
 */

#include "SevenSegDriver.h"
#include "../Config.h"

// 静的メンバ変数の定義
const int SevenSegDriver::placePin[] = {PIN_D10, PIN_D11, PIN_D12, PIN_D13};
const int SevenSegDriver::colonPin[] = {PIN_D03, PIN_D04, PIN_D05, PIN_D06, PIN_D07};

// 7セグメントのデータ
// 0, 1, 2, ... 9, ハイフン
// 左からabcdefg, dotに対応する
const unsigned char SevenSegDriver::number[] = {0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90, 0xbf};
const unsigned char SevenSegDriver::numdot[] = {0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02, 0x78, 0x00, 0x10, 0xbf};

void SevenSegDriver::begin() {
    Serial.begin(115200);
    Serial.println("SevenSegDriver initialized");
}

void SevenSegDriver::clear() {
    Serial.println("[7SEG] CLEAR");
}

void SevenSegDriver::pickDigit(int digit) {
    for (int i = 0; i < 4; i++) {
        digitalWrite(placePin[i], LOW);  // 全てOFF
    }
    digitalWrite(placePin[digit], HIGH);  // 指定の桁だけON

    for (int i = 0; i < 4; i++) {
        digitalWrite(colonPin[i], LOW);  // 全てOFF
    }
    digitalWrite(colonPin[digit], HIGH);  // 指定の桁だけON
}

// シフトレジスタにデータを送る関数
void SevenSegDriver::hc595_shift(byte data) {
    digitalWrite(RCLK_PIN, LOW);  // 送信準備

    // MSBFIRST: 最上位ビットから順に送信
    shiftOut(SDI_PIN, SRCLK_PIN, MSBFIRST, data);
    shiftOut(SDI_PIN, SRCLK_PIN, MSBFIRST, data);

    digitalWrite(RCLK_PIN, HIGH);  // ラッチ(出力反映)
}

// 表示をクリアする関数
void SevenSegDriver::clearDisplay() {
    // 0xFF (すべて1) を送ると、Common Anodeの場合は全消灯
    hc595_shift(0xFF);
}

void SevenSegDriver::show(DisplayDataType type, const char* value) {
    // ピンモード設定
    pinMode(SDI_PIN, OUTPUT);
    pinMode(RCLK_PIN, OUTPUT);
    pinMode(SRCLK_PIN, OUTPUT);

    for (int i = 0; i < 4; i++) {
        pinMode(placePin[i], OUTPUT);
        digitalWrite(placePin[i], LOW);  // 初期状態はOFFにしておく(High ActiveならLOWでOFF)
    }

    for (int i = 0; i < 5; i++) {
        pinMode(colonPin[i], OUTPUT);
        digitalWrite(colonPin[i], LOW);  // 初期状態はOFFにしておく(High ActiveならLOWでOFF)
    }

    // --- ダイナミック点灯処理 (高速切り替え) ---
    // atofを使って、文字列を一度「小数(float)」に変換する
    float f_val = atof(value);

    // Serial.println(f_val);
    // その数値を100倍して整数に入れる
    molding = (int)(f_val * 100);
    dig1 = (molding / 1000);

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

    #ifdef DEBUGDAO
        if(type==0){
            dig1 = 1;
            dig2=2;
            dig3=3;
            dig4=4;
        }
    #endif

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
