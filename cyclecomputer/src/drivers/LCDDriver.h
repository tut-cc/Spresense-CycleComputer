/*
 * ファイル: LCDDriver.h
 * 説明: 標準的な 16x2 テキスト LCD 用のディスプレイドライバー。
 *       サイクルコンピューターデータを表示するための IDisplay インターフェースを実装します。
 */

#ifndef LCDDRIVER_H
#define LCDDRIVER_H

#include "../interfaces/IDisplay.h"
#include "../Config.h"

#include <LiquidCrystal.h>

// Config.h で定義されたピン

class LCDDriver : public IDisplay {
private:
    LiquidCrystal lcd;

public:
    LCDDriver()
        : lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7) {}

    void begin() override {
        lcd.begin(16, 2);
    }

    void clear() override {
        lcd.clear();
    }

    void show(DisplayDataType type, const char* value) override {
        clear();
        lcd.setCursor(0, 0); // 1行目
        
        switch (type) {
            case DISPLAY_DATA_SPEED:
                lcd.print("Speed");
                lcd.setCursor(0, 1);
                lcd.print(value);
                lcd.print(" km/h");
                break;
            case DISPLAY_DATA_TIME:
                lcd.print("Time (JST)");
                lcd.setCursor(0, 1);
                lcd.print(value);
                break;
            case DISPLAY_DATA_MAX_SPEED:
                lcd.print("Max Speed");
                lcd.setCursor(0, 1);
                lcd.print(value);
                lcd.print(" km/h");
                break;
            case DISPLAY_DATA_DISTANCE:
                lcd.print("Distance");
                lcd.setCursor(0, 1);
                lcd.print(value);
                lcd.print(" km");
                break;
            case DISPLAY_DATA_MOVING_TIME:
                lcd.print("Moving Time");
                lcd.setCursor(0, 1);
                lcd.print(value);
                break;
            case DISPLAY_DATA_ELAPSED_TIME:
                lcd.print("Total Time");
                lcd.setCursor(0, 1);
                lcd.print(value);
                break;
            case DISPLAY_DATA_AVG_SPEED:
                lcd.print("Avg Speed");
                lcd.setCursor(0, 1);
                lcd.print(value);
                lcd.print(" km/h");
                break;
        }
    }
};

#endif
