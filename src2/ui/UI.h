#pragma once

#include "../common/DataStructures.h"
#include "../hardware/OLED.h"
#include "Input.h"
#include "Renderer.h"

constexpr int BTN_A = PIN_D09;
constexpr int BTN_B = PIN_D04;

class UI {
private:
  OLED     oled;
  Input    input;
  Renderer renderer;

public:
  UI() : input(BTN_A, BTN_B) {}

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
    oled.setCursor((WIDTH - rect.w) / 2, (HEIGHT - rect.h) / 2);
    oled.print(msg);
    oled.display();
    delay(500);
    oled.restart();
  }
};
