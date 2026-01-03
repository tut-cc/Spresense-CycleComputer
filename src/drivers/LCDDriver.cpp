#include "LCDDriver.h"

#include "../Config.h"

LCDDriver::LCDDriver() : lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7) {}

void LCDDriver::begin() {
    lcd.begin(16, 2);
}

void LCDDriver::clear() {
    lcd.clear();
}

void LCDDriver::show(DisplayDataType type, const char* value) {
    clear();
    lcd.setCursor(0, 0);  // 1行目

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
