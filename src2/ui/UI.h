#pragma once

#include "../DataStructures.h"
#include "../hardware/OLED.h"
#include "Input.h"
#include "Mode.h"
#include "Renderer.h"

constexpr int BTN_A = PIN_D09;
constexpr int BTN_B = PIN_D04;

class UI {
private:
  OLED         oled;
  Input        input;
  Renderer     renderer;
  DisplayFrame frames[2];
  int          currentIdx = 0;

  DisplayFrame createFrame(const DisplayData &displayData) const {
    DisplayFrame frame;

    switch (displayData.fixMode) {
    case Fix2D:
      strcpy(frame.header.fixStatus, "2D");
      break;
    case Fix3D:
      strcpy(frame.header.fixStatus, "3D");
      break;
    default:
      strcpy(frame.header.fixStatus, "WAIT");
      break;
    }

    Mode::fillFrame(frame, displayData);

    return frame;
  }

public:
  UI() : input(BTN_A, BTN_B) {}

  void begin() {
    oled.begin();
    input.begin();
  }

  // 入力を取得する
  Input::Event getInputEvent() {
    return input.update();
  }

  // 表示を更新する
  void draw(const DisplayData &displayData) {

    const int prevIdx = currentIdx;
    currentIdx        = 1 - currentIdx;

    frames[currentIdx] = createFrame(displayData);

    if (frames[currentIdx] != frames[prevIdx]) { renderer.render(oled, frames[currentIdx]); }
  }

  // 演出用
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
    frames[0] = DisplayFrame();
    frames[1] = DisplayFrame();
  }
};
