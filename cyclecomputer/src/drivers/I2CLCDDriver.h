/*
 * ファイル: I2CLCDDriver.h
 * 説明: I2C接続のLCD用ディスプレイドライバー。
 */

#ifndef I2CLCDDRIVER_H
#define I2CLCDDRIVER_H

#include "../interfaces/IDisplay.h"
#include "../Config.h"
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

class I2CLCDDriver : public IDisplay {
private:
    LiquidCrystal_I2C lcd;
    char _shadowLine1[17];
    char _shadowLine2[17];

public:
    I2CLCDDriver()
        : lcd(LCD_I2C_ADDR, LCD_COLS, LCD_ROWS) {
            memset(_shadowLine1, 0, sizeof(_shadowLine1));
            memset(_shadowLine2, 0, sizeof(_shadowLine2));
        }

    void begin() override {
        // I2Cの安定性を向上させるため、速度を下げる
        // 通常の100kHzから50kHzへ
        lcd.init();
        lcd.backlight();
        Wire.setClock(50000); 
    }

    void clear() override {
        lcd.clear();
        memset(_shadowLine1, 0, sizeof(_shadowLine1));
        memset(_shadowLine2, 0, sizeof(_shadowLine2));
    }

    void show(DisplayDataType type, const char* value) override {
        char nextLine1[17];
        char nextLine2[17];
        
        const char* label = "";
        const char* suffix = "";
        
        switch (type) {
            case DISPLAY_DATA_SPEED:        label = "Speed";        suffix = " km/h"; break;
            case DISPLAY_DATA_TIME:         label = "Time (JST)";   suffix = ""; break;
            case DISPLAY_DATA_MAX_SPEED:    label = "Max Speed";    suffix = " km/h"; break;
            case DISPLAY_DATA_DISTANCE:     label = "Distance";     suffix = " km"; break;
            case DISPLAY_DATA_MOVING_TIME:  label = "Moving Time";  suffix = ""; break;
            case DISPLAY_DATA_ELAPSED_TIME: label = "Total Time";   suffix = ""; break;
            case DISPLAY_DATA_AVG_SPEED:    label = "Avg Speed";    suffix = " km/h"; break;
        }

        // 行1の構築: ラベルを左詰め、余白はスペースで埋める (16文字固定)
        snprintf(nextLine1, sizeof(nextLine1), "%-16s", label);

        // 行2の構築: 値 + 接尾辞
        char tempBuf[25]; // 余裕を持たせる
        snprintf(tempBuf, sizeof(tempBuf), "%s%s", value, suffix);
        snprintf(nextLine2, sizeof(nextLine2), "%-16s", tempBuf);

        // 差分がある場合のみ更新
        updateLine(0, _shadowLine1, nextLine1);
        updateLine(1, _shadowLine2, nextLine2);
    }

private:
    // ヘルパー: 行更新
    void updateLine(int row, char* shadow, const char* newContent) {
        if (strncmp(shadow, newContent, 16) != 0) {
            lcd.setCursor(0, row);
            lcd.print(newContent); 
            memcpy(shadow, newContent, 16);
        }
    }
};

#endif
