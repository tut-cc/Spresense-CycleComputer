#pragma once

#include "../Config.h"
#include "../domain/TripData.h"
#include <Arduino.h>
#include <cstring>
#include <stdio.h>

enum class Mode { SPD_TIM, AVG_ODO, MAX_CLK };

struct Header {
  const char *fixStatus = "";
  const char *modeSpeed = "";
  const char *modeTime  = "";

  bool operator==(const Header &other) const {
    return fixStatus == other.fixStatus && modeSpeed == other.modeSpeed &&
           modeTime == other.modeTime;
  }
};

struct Item {
  char        value[16] = {0};
  const char *unit      = "";

  bool operator==(const Item &other) const {
    return strcmp(value, other.value) == 0 && unit == other.unit;
  }
};

struct DisplayFrame {
  Header header;
  Item   main, sub;

  DisplayFrame() = default;

  DisplayFrame(const TripData &state, const GnssData &gnss, const RtcTime &clock, Mode mode) {
    static const char *FIX_LABELS[] = {"WAIT", "2D", "3D"};
    const SpFixMode    fixMode      = (SpFixMode)gnss.navData.posFixMode;
    header.fixStatus = (fixMode >= 1 && fixMode <= 3) ? FIX_LABELS[fixMode - 1] : FIX_LABELS[0];

    struct ModeConfiguration {
      const char *speedLabel, *timeLabel, *mainUnit, *subUnit;
    };

    static const ModeConfiguration MODE_CONFIGS[] = {
        {"SPD", "Time", "km/h", ""},
        {"AVG", "Odo", "km/h", "km"},
        {"MAX", "Clock", "km/h", ""},
    };

    const auto &modeConfig = MODE_CONFIGS[(int)mode];
    header.modeSpeed       = modeConfig.speedLabel;
    header.modeTime        = modeConfig.timeLabel;

    main.unit = modeConfig.mainUnit;
    sub.unit  = modeConfig.subUnit;

    const bool paused = state.isPaused() && ((millis() / Config::UI::BLINK_INTERVAL_MS) % 2 == 0);

    switch (mode) {
    case Mode::SPD_TIM:
      snprintf(main.value, sizeof(main.value), "%4.1f", state.speed.current);
      if (paused) strcpy(sub.value, ""), sub.unit = "";
      else {
        unsigned long totalSeconds = state.time.elapsed / 1000;
        unsigned long hours        = totalSeconds / 3600;
        unsigned long minutes      = (totalSeconds % 3600) / 60;
        unsigned long seconds      = totalSeconds % 60;
        if (hours > 0)
          snprintf(sub.value, sizeof(sub.value), "%lu:%02lu:%02lu", hours, minutes, seconds);
        else snprintf(sub.value, sizeof(sub.value), "%02lu:%02lu", minutes, seconds);
      }
      return;

    case Mode::AVG_ODO:
      snprintf(main.value, sizeof(main.value), "%4.1f", state.speed.avg);
      snprintf(sub.value, sizeof(sub.value), "%5.2f", state.distance);
      return;

    case Mode::MAX_CLK:
      snprintf(main.value, sizeof(main.value), "%4.1f", state.speed.max);
      int displayHour = (clock.year() >= Config::Time::MIN_VALID_YEAR)
                            ? (clock.hour() + Config::Time::TIMEZONE_OFFSET_HOURS) % 24
                            : clock.hour();
      snprintf(sub.value, sizeof(sub.value), "%02d:%02d", displayHour, clock.minute());
      return;
    }
  }

  bool operator==(const DisplayFrame &other) const {
    return header == other.header && main == other.main && sub == other.sub;
  }

  bool operator!=(const DisplayFrame &other) const { return !(*this == other); }
};
