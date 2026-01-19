#pragma once
/**
 * @file Renderer.h
 * @brief OLED画面への描画クラス
 *
 * DisplayFrameの内容をOLEDディスプレイに描画します。
 *
 * 画面レイアウト:
 * ┌──────────────┐
 * │ WAIT    SPD    Time       │ ← ヘッダー(GPS状態/モード)
 * │──────────────│ ← 区切り線
 * │       23.5 km/h           │ ← メイン表示(速度)
 * │                           │
 * │         12:34             │ ← サブ表示(時間/距離/時刻)
 * └──────────────┘
 */

#include <GNSS.h>
#include <cstring>

#include "../common/DataStructures.h"
#include "../hardware/OLED.h"

// レイアウト定数
constexpr int16_t HEADER_HEIGHT        = 12; ///< ヘッダー高さ
constexpr int16_t HEADER_TEXT_SIZE     = 1;  ///< ヘッダー文字サイズ
constexpr int16_t HEADER_LINE_Y_OFFSET = 2;  ///< 区切り線のY位置調整
constexpr int16_t MAIN_AREA_Y_OFFSET   = 14; ///< メイン表示領域の開始Y
constexpr int16_t MAIN_VAL_SIZE        = 3;  ///< メイン値の文字サイズ
constexpr int16_t MAIN_UNIT_SIZE       = 1;  ///< メイン単位の文字サイズ
constexpr int16_t SUB_VAL_SIZE         = 2;  ///< サブ値の文字サイズ
constexpr int16_t SUB_UNIT_SIZE        = 1;  ///< サブ単位の文字サイズ
constexpr int16_t UNIT_SPACING         = 4;  ///< 値と単位の間隔

/**
 * @class Renderer
 * @brief 画面描画を担当
 */
class Renderer {
public:
  Renderer() {}

  /**
   * @brief フレームを描画
   * @param oled OLEDディスプレイ
   * @param frame 描画するフレーム
   */
  void render(OLED &oled, const DisplayFrame &frame) {
    oled.clear();
    drawHeader(oled, frame);
    drawMainArea(oled, frame);
    oled.display();
  }

private:
  /** @brief ヘッダー部分を描画 */
  void drawHeader(OLED &oled, const DisplayFrame &frame) {
    oled.setTextSize(HEADER_TEXT_SIZE);
    oled.setTextColor(WHITE);

    // 左: GPS状態, 中央: 速度モード, 右: 時間モード
    drawTextLeft(oled, 0, frame.header.fixStatus);
    drawTextCenter(oled, 0, frame.header.modeSpeed);
    drawTextRight(oled, 0, frame.header.modeTime);

    // 区切り線
    int16_t lineY = HEADER_HEIGHT - HEADER_LINE_Y_OFFSET;
    oled.drawLine(0, lineY, oled.getWidth(), lineY, WHITE);
  }

  /** @brief メイン領域を描画 */
  void drawMainArea(OLED &oled, const DisplayFrame &frame) {
    const int16_t headerH = HEADER_HEIGHT;
    const int16_t screenH = oled.getHeight();

    // メイン表示（速度）- 画面中央上部
    drawItem(oled, frame.main, headerH + MAIN_AREA_Y_OFFSET, MAIN_VAL_SIZE, MAIN_UNIT_SIZE, false);
    // サブ表示（時間等）- 画面下部
    drawItem(oled, frame.sub, screenH, SUB_VAL_SIZE, SUB_UNIT_SIZE, true);
  }

  /**
   * @brief 値+単位のペアを描画
   * @param alignBottom true:下端揃え, false:中央揃え
   */
  void drawItem(OLED &oled, const DisplayFrame::Item &item, int16_t y, uint8_t valSize,
                uint8_t unitSize, bool alignBottom) {
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

    // 水平方向は中央揃え
    const int16_t startX = (oled.getWidth() - totalW) / 2;
    // 垂直位置を計算
    const int16_t valY  = alignBottom ? (y - valRect.h) : (y - valRect.h / 2);
    const int16_t unitY = alignBottom ? (y - unitRect.h) : (y + valRect.h / 2 - unitRect.h);

    // 値を描画
    oled.setTextSize(valSize);
    oled.setCursor(startX, valY);
    oled.print(item.value);

    if (!hasUnit) return;

    // 単位を描画
    oled.setTextSize(unitSize);
    oled.setCursor(startX + valRect.w + UNIT_SPACING, unitY);
    oled.print(item.unit);
  }

  void drawTextLeft(OLED &oled, int16_t y, const char *text) {
    oled.setCursor(0, y);
    oled.print(text);
  }

  void drawTextCenter(OLED &oled, int16_t y, const char *text) {
    OLED::Rect rect = oled.getTextBounds(text);
    oled.setCursor((oled.getWidth() - rect.w) / 2, y);
    oled.print(text);
  }

  void drawTextRight(OLED &oled, int16_t y, const char *text) {
    OLED::Rect rect = oled.getTextBounds(text);
    oled.setCursor(oled.getWidth() - rect.w, y);
    oled.print(text);
  }
};
