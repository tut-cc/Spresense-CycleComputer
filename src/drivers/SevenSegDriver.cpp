#include "SevenSegDriver.h"

#include "../Config.h"

// 静的メンバ変数の定義
const int SevenSegDriver::placePin[] = {Config::Pin::D1, Config::Pin::D2, Config::Pin::D3,
                                        Config::Pin::D4};
const int SevenSegDriver::colonPin[] = {Config::Pin::D1_COLON, Config::Pin::D2_COLON,
                                        Config::Pin::D3_COLON, Config::Pin::D4_COLON,
                                        Config::Pin::DOT_COLON};

// 7セグメントのデータ
// Configから参照
const unsigned char* SevenSegDriver::number = Config::SevenSeg::NUMBERS;
const unsigned char* SevenSegDriver::numdot = Config::SevenSeg::NUMBERS_WITH_DOT;

void SevenSegDriver::begin() {
    Serial.begin(115200);
    Serial.println("[SevenSegDriver] initialized");
}

void SevenSegDriver::clear() {
    Serial.println("[SevenSegDriver] CLEAR");
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
    digitalWrite(Config::Pin::RCLK, LOW);  // 送信準備

    // MSBFIRST: 最上位ビットから順に送信
    shiftOut(Config::Pin::SDI, Config::Pin::SRCLK, MSBFIRST, data);
    shiftOut(Config::Pin::SDI, Config::Pin::SRCLK, MSBFIRST, data);

    digitalWrite(Config::Pin::RCLK, HIGH);  // ラッチ(出力反映)
}

// 表示をクリアする関数
void SevenSegDriver::clearDisplay() {
    // 0xFF (すべて1) を送ると、Common Anodeの場合は全消灯
    hc595_shift(0xFF);
}

void SevenSegDriver::show(DisplayDataType type, const char* value) {
    (void)type;  // unused parameter
    pinMode(Config::Pin::SDI, OUTPUT);
    pinMode(Config::Pin::RCLK, OUTPUT);
    pinMode(Config::Pin::SRCLK, OUTPUT);

    for (int i = 0; i < 4; i++) {
        pinMode(placePin[i], OUTPUT);
        digitalWrite(placePin[i], LOW);  // 初期状態はOFFにしておく(Active HighならLOWでOFF)
    }

    for (int i = 0; i < 5; i++) {
        pinMode(colonPin[i], OUTPUT);
        digitalWrite(colonPin[i], LOW);  // 初期状態はOFFにしておく(Active HighならLOWでOFF)
    }

    // --- ダイナミック点灯処理 (高速切り替え) ---

#ifdef DEBUGDAO
    dig1 = 1;
    dig2 = 2;
    dig3 = 3;
    dig4 = 4;
#else
    // atofを使って、文字列を一度「少数(float)」に変換する
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
#endif
    // Serial.print("[SevenSegDriver] ");
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
