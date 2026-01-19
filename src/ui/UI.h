#pragma once

#include "../hardware/OLED.h"
#include "Input.h"
#include "Mode.h"
#include "Renderer.h"

constexpr int BTN_A = PIN_D09;
constexpr int BTN_B = PIN_D04;

class UI {
private:
  OLED     oled;
  Input    input;
  Mode     mode;
  Renderer renderer;

  Frame createFrame(const SpNavData &navData, const Trip &trip, const Clock &clock) const {
    Frame frame;

    switch (navData.posFixMode) {
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

    mode.fillFrame(frame, trip, clock);

    return frame;
  }

public:
  UI() : input(BTN_A, BTN_B) {}

  void begin() {
    oled.begin();
    input.begin();
  }

  void update(Trip &trip, DataStore &dataStore, const Clock &clock, const SpNavData &navData) {
    Input::Event id = input.update();

    if (id == Input::Event::RESET_LONG) {
      oled.clear();
      oled.setTextSize(1);
      oled.setTextColor(WHITE);
      const char *msg  = "RESETTING...";
      OLED::Rect  rect = oled.getTextBounds(msg);
      oled.setCursor((oled.getWidth() - rect.w) / 2, (oled.getHeight() - rect.h) / 2);
      oled.print(msg);
      oled.display();
      delay(500); // Visual feedback

      oled.restart();
      renderer.reset();
    }

    if (id != Input::Event::NONE) { mode.handleInput(id, trip, dataStore); }

    Frame frame = createFrame(navData, trip, clock);
    renderer.render(oled, frame);
  }
};
