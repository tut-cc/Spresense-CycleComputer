#pragma once

/**
 * @file Renderer.h
 * @brief DisplayFrameのOLEDへの描画処理
 *
 * DisplayFrame構造体の内容をOLEDに描画します。
 * ヘッダー、メイン表示、サブ表示の各エリアのレイアウトを担当。
 */

#include <GNSS.h>
#include <cstring>

#include "../common/Config.h"
#include "../hardware/OLED.h"
#include "DisplayFrame.h"

/// 表示レイアウト定数
constexpr int16_t HEADER_HEIGHT        = 12;
constexpr int16_t HEADER_TEXT_SIZE     = 1;
constexpr int16_t HEADER_LINE_Y_OFFSET = 2;
constexpr int16_t MAIN_AREA_Y_OFFSET   = 14;
constexpr int16_t MAIN_VAL_SIZE        = 3;
constexpr int16_t MAIN_UNIT_SIZE       = 1;
constexpr int16_t SUB_VAL_SIZE         = 2;
constexpr int16_t SUB_UNIT_SIZE        = 1;
constexpr int16_t UNIT_SPACING         = 4;

class Renderer {
private:
  OLED oled;

public:
  Renderer() {}

  bool begin() { return oled.begin(); }

  void render(const DisplayFrame &frame) {
    oled.clear();
    drawHeader(frame);
    drawMainArea(frame);
    oled.display();
  }

  void showResetMessage() {
    oled.clear();
    oled.setTextSize(2);
    oled.setTextColor(WHITE);
    const char *msg  = "RESETTING...";
    OLED::Rect  rect = oled.getTextBounds(msg);
    oled.setCursor((Config::Display::WIDTH - rect.w) / 2, (Config::Display::HEIGHT - rect.h) / 2);
    oled.print(msg);
    oled.display();
    delay(500);
    oled.restart();
  }

private:
  void drawHeader(const DisplayFrame &frame) {
    oled.setTextSize(HEADER_TEXT_SIZE);
    oled.setTextColor(WHITE);

    drawTextLeft(0, frame.header.fixStatus);
    drawTextCenter(0, frame.header.modeSpeed);
    drawTextRight(0, frame.header.modeTime);

    int16_t lineY = HEADER_HEIGHT - HEADER_LINE_Y_OFFSET;
    oled.drawLine(0, lineY, Config::Display::WIDTH, lineY, WHITE);
  }

  void drawMainArea(const DisplayFrame &frame) {
    const int16_t headerH = HEADER_HEIGHT;
    const int16_t screenH = Config::Display::HEIGHT;

    drawItem(frame.main, headerH + MAIN_AREA_Y_OFFSET, MAIN_VAL_SIZE, MAIN_UNIT_SIZE, false);
    drawItem(frame.sub, screenH, SUB_VAL_SIZE, SUB_UNIT_SIZE, true);
  }

  void drawItem(const DisplayFrame::Item &item, int16_t y, uint8_t valSize, uint8_t unitSize,
                bool alignBottom) {
    oled.setTextSize(valSize);
    OLED::Rect valRect = oled.getTextBounds(item.value);

    const bool hasUnit  = (strlen(item.unit) > 0);
    int16_t    totalW   = valRect.w;
    OLED::Rect unitRect = {0, 0, 0, 0};

    if (hasUnit) {
      oled.setTextSize(unitSize);
      unitRect = oled.getTextBounds(item.unit);
      totalW += UNIT_SPACING + unitRect.w;
    }

    const int16_t startX = (Config::Display::WIDTH - totalW) / 2;
    const int16_t valY   = alignBottom ? (y - valRect.h) : (y - valRect.h / 2);
    const int16_t unitY  = alignBottom ? (y - unitRect.h) : (y + valRect.h / 2 - unitRect.h);

    oled.setTextSize(valSize);
    oled.setCursor(startX, valY);
    oled.print(item.value);

    if (!hasUnit) return;

    oled.setTextSize(unitSize);
    oled.setCursor(startX + valRect.w + UNIT_SPACING, unitY);
    oled.print(item.unit);
  }

  void drawTextLeft(int16_t y, const char *text) {
    oled.setCursor(0, y);
    oled.print(text);
  }

  void drawTextCenter(int16_t y, const char *text) {
    OLED::Rect rect = oled.getTextBounds(text);
    oled.setCursor((Config::Display::WIDTH - rect.w) / 2, y);
    oled.print(text);
  }

  void drawTextRight(int16_t y, const char *text) {
    OLED::Rect rect = oled.getTextBounds(text);
    oled.setCursor(Config::Display::WIDTH - rect.w, y);
    oled.print(text);
  }
};
