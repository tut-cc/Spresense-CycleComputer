#pragma once

/**
 * @file UI.h
 * @brief ユーザーインターフェース統合クラス
 *
 * OLED表示、ボタン入力、レンダリングを統合し、
 * 一つのUIモジュールとして提供します。
 */

#include "../common/Config.h"
#include "../common/DataStructures.h"
#include "../hardware/OLED.h"
#include "Input.h"
#include "Renderer.h"

class UI {
private:
  OLED     oled;
  Input    input;
  Renderer renderer;

public:
  UI() : input(Config::Pins::BUTTON_SELECT, Config::Pins::BUTTON_PAUSE) {}

  void begin() {
    oled.begin();
    input.begin();
  }

  Input::Event getInputEvent() { return input.update(); }

  void draw(const DisplayFrame &frame) { renderer.render(oled, frame); }

  void showResetMessage() {
    oled.clear();
    oled.setTextSize(1);
    oled.setTextColor(WHITE);
    const char *msg  = "RESETTING...";
    OLED::Rect  rect = oled.getTextBounds(msg);
    oled.setCursor((Config::Display::WIDTH - rect.w) / 2, (Config::Display::HEIGHT - rect.h) / 2);
    oled.print(msg);
    oled.display();
    delay(500);
    oled.restart();
  }
};
