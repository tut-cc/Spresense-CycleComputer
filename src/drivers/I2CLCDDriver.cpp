#include "I2CLCDDriver.h"

#include "../Config.h"

I2CLCDDriver::I2CLCDDriver() : lcd(LCD_I2C_ADDR, LCD_COLS, LCD_ROWS) {
    memset(shadowLine1, 0, sizeof(shadowLine1));
    memset(shadowLine2, 0, sizeof(shadowLine2));
}

void I2CLCDDriver::begin() {
    // I2Cの安定性を向上させるため、速度を下げる
    // 通常の100kHzから50kHzへ
    lcd.init();
    lcd.backlight();
    Wire.setClock(50000);
}

void I2CLCDDriver::clear() {
    lcd.clear();
    memset(shadowLine1, 0, sizeof(shadowLine1));
    memset(shadowLine2, 0, sizeof(shadowLine2));
}

void I2CLCDDriver::show(DisplayDataType type, const char *value) {
    char nextLine1[17];
    char nextLine2[17];

    const char *label = "";
    const char *suffix = "";

    switch (type) {
        case DISPLAY_DATA_SPEED:
            label = "Speed";
            suffix = " km/h";
            break;
        case DISPLAY_DATA_TIME:
            label = "Time (JST)";
            suffix = "";
            break;
        case DISPLAY_DATA_MAX_SPEED:
            label = "Max Speed";
            suffix = " km/h";
            break;
        case DISPLAY_DATA_DISTANCE:
            label = "Distance";
            suffix = " km";
            break;
        case DISPLAY_DATA_MOVING_TIME:
            label = "Moving Time";
            suffix = "";
            break;
        case DISPLAY_DATA_ELAPSED_TIME:
            label = "Total Time";
            suffix = "";
            break;
        case DISPLAY_DATA_AVG_SPEED:
            label = "Avg Speed";
            suffix = " km/h";
            break;
    }

    // 行1の構築: ラベルを左詰め、余白はスペースで埋める (16文字固定)
    snprintf(nextLine1, sizeof(nextLine1), "%-16s", label);

    // 行2の構築: 値 + 接尾辞
    char tempBuf[25];  // 余裕を持たせる
    snprintf(tempBuf, sizeof(tempBuf), "%s%s", value, suffix);
    snprintf(nextLine2, sizeof(nextLine2), "%-16s", tempBuf);

    // 差分がある場合のみ更新
    updateLine(0, shadowLine1, nextLine1);
    updateLine(1, shadowLine2, nextLine2);
}

// ヘルパー: 行更新
void I2CLCDDriver::updateLine(int row, char *shadow, const char *newContent) {
    if (strncmp(shadow, newContent, 16) != 0) {
        lcd.setCursor(0, row);
        lcd.print(newContent);
        memcpy(shadow, newContent, 16);
    }
}
