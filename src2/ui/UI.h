#pragma once
/**
 * @file UI.h
 * @brief ユーザーインターフェース統合クラス
 *
 * OLED表示とボタン入力を統合して管理します。
 * App層から見たUIへの単一の窓口となります。
 */

#include "../common/DataStructures.h"
#include "../hardware/OLED.h"
#include "Input.h"
#include "Renderer.h"

constexpr int BTN_A = PIN_D09; ///< SELECTボタンのピン
constexpr int BTN_B = PIN_D04; ///< PAUSEボタンのピン

/**
 * @class UI
 * @brief ユーザーインターフェース
 */
class UI {
private:
  OLED     oled;     ///< OLEDディスプレイ
  Input    input;    ///< 2ボタン入力
  Renderer renderer; ///< 画面描画

public:
  UI() : input(BTN_A, BTN_B) {}

  /** @brief 初期化 */
  void begin() {
    oled.begin();
    input.begin();
  }

  /**
   * @brief 入力イベントを取得
   * @return 発生したイベント（なければNONE）
   */
  Input::Event getInputEvent() { return input.update(); }

  /**
   * @brief フレームを描画
   * @param frame 描画するフレーム
   */
  void draw(const DisplayFrame &frame) { renderer.render(oled, frame); }

  /**
   * @brief リセット中メッセージを表示
   *
   * 全データリセット時に500ms間表示します。
   */
  void showResetMessage() {
    oled.clear();
    oled.setTextSize(1);
    oled.setTextColor(WHITE);
    const char *msg  = "RESETTING...";
    OLED::Rect  rect = oled.getTextBounds(msg);
    oled.setCursor((oled.getWidth() - rect.w) / 2, (oled.getHeight() - rect.h) / 2);
    oled.print(msg);
    oled.display();
    delay(500);
    oled.restart();
  }
};
