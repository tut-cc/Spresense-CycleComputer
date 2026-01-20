#pragma once

#include "../Config.h"
#include "../domain/TripData.h"
#include <Arduino.h>
#include <cstring>
#include <stdio.h>

enum class Mode { SPD_TIM, AVG_ODO, MAX_CLK };

struct DisplayFrame {
  struct Header {
    const char *fixStatus = "";
    const char *modeSpeed = "";
    const char *modeTime  = "";

    bool operator==(const Header &o) const {
      return fixStatus == o.fixStatus && modeSpeed == o.modeSpeed && modeTime == o.modeTime;
    }
  } header;

  struct Item {
    char        value[16] = {0};
    const char *unit      = "";

    bool operator==(const Item &o) const { return strcmp(value, o.value) == 0 && unit == o.unit; }
  } main, sub;

  DisplayFrame() = default;

  DisplayFrame(const TripData &state, const GnssData &gnss, const SpGnssTime &clock, Mode mode) {
    static const char *FIX_LABELS[] = {"WAIT", "2D", "3D"};
    const SpFixMode    fixMode      = (SpFixMode)gnss.navData.posFixMode;
    header.fixStatus = (fixMode >= 1 && fixMode <= 3) ? FIX_LABELS[fixMode - 1] : FIX_LABELS[0];

    struct ModeCfg {
      const char *s, *t, *mu, *su;
    };

    static const ModeCfg CFG[] = {
        {"SPD", "Time", "km/h", ""},
        {"AVG", "Odo", "km/h", "km"},
        {"MAX", "Clock", "km/h", ""},
    };

    const auto &c    = CFG[(int)mode];
    header.modeSpeed = c.s;
    header.modeTime  = c.t;

    main.unit = c.mu;
    sub.unit  = c.su;

    const bool paused = state.isPaused() && ((millis() / Config::UI::BLINK_INTERVAL_MS) % 2 == 0);

    switch (mode) {
    case Mode::SPD_TIM:
      snprintf(main.value, sizeof(main.value), "%4.1f", state.speed.current);
      if (paused) strcpy(sub.value, ""), sub.unit = "";
      else {
        unsigned long sec = state.time.elapsed / 1000, h = sec / 3600, m = (sec % 3600) / 60,
                      sc = sec % 60;
        if (h > 0) snprintf(sub.value, sizeof(sub.value), "%lu:%02lu:%02lu", h, m, sc);
        else snprintf(sub.value, sizeof(sub.value), "%02lu:%02lu", m, sc);
      }
      return;

    case Mode::AVG_ODO:
      snprintf(main.value, sizeof(main.value), "%4.1f", state.speed.avg);
      snprintf(sub.value, sizeof(sub.value), "%5.2f", state.distance.total);
      return;

    case Mode::MAX_CLK:
      snprintf(main.value, sizeof(main.value), "%4.1f", state.speed.max);
      int h = (clock.year >= Config::Time::MIN_VALID_YEAR)
                  ? (clock.hour + Config::Time::TIMEZONE_OFFSET_HOURS) % 24
                  : clock.hour;
      snprintf(sub.value, sizeof(sub.value), "%02d:%02d", h, clock.minute);
      return;
    }
  }

  bool operator==(const DisplayFrame &o) const {
    return header == o.header && main == o.main && sub == o.sub;
  }
  bool operator!=(const DisplayFrame &o) const { return !(*this == o); }
};
