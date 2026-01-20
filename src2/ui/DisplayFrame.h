#pragma once

/**
 * @file DisplayFrame.h
 * @brief OLEDに表示する1フレームのデータ構造と構築ロジック
 */

#include "../common/Config.h"
#include "../common/Formatter.h"
#include "../domain/TripState.h"
#include <Arduino.h>
#include <cstring>

struct DisplayFrame {
  struct Header {
    const char *fixStatus;
    const char *modeSpeed;
    const char *modeTime;

    Header() : fixStatus(""), modeSpeed(""), modeTime("") {}

    bool operator==(const Header &other) const {
      return (fixStatus == other.fixStatus) && (modeSpeed == other.modeSpeed) && (timeMatch(other));
    }
    bool operator!=(const Header &other) const { return !(*this == other); }

  private:
    bool timeMatch(const Header &other) const { return modeTime == other.modeTime; }
  };

  struct Item {
    char        value[16];
    const char *unit;

    Item() : unit("") { memset(value, 0, sizeof(value)); }

    bool operator==(const Item &other) const {
      return (strcmp(value, other.value) == 0) && (unit == other.unit);
    }
    bool operator!=(const Item &other) const { return !(*this == other); }
  };

  Header header;
  Item   main;
  Item   sub;

  DisplayFrame() = default;

  DisplayFrame(const TripState &state, const GnssData &gnss, const SpGnssTime &currentTime,
               Mode mode) {
    struct ModeConfig {
      const char *speedLabel;
      const char *timeLabel;
      const char *mainUnit;
      const char *subUnit;
    };

    static const ModeConfig CONFIGS[] = {
        {"SPD", "Time", "km/h", ""},
        {"AVG", "Odo", "km/h", "km"},
        {"MAX", "Clock", "km/h", ""},
    };

    static const char *FIX_LABELS[] = {"WAIT", "2D", "3D"};

    const ModeConfig &cfg = CONFIGS[(int)mode];

    const SpFixMode fixMode = (SpFixMode)gnss.navData.posFixMode;
    if (fixMode == Fix3D) header.fixStatus = FIX_LABELS[2];
    else if (fixMode == Fix2D) header.fixStatus = FIX_LABELS[1];
    else header.fixStatus = FIX_LABELS[0];

    header.modeSpeed = cfg.speedLabel;
    header.modeTime  = cfg.timeLabel;

    const bool isBlinkPhase =
        state.isPaused() && ((millis() / Config::UI::BLINK_INTERVAL_MS) % 2 == 0);
    const bool shouldBlink = (mode == Mode::SPD_TIM) && isBlinkPhase;

    switch (mode) {
    case Mode::SPD_TIM:
      Formatter::formatSpeed(state.speed.current, main.value, sizeof(main.value));
      main.unit = cfg.mainUnit;
      if (shouldBlink) {
        strcpy(sub.value, "");
        sub.unit = "";
      } else {
        Formatter::formatDuration(state.time.elapsed, sub.value, sizeof(sub.value));
        sub.unit = cfg.subUnit;
      }
      break;

    case Mode::AVG_ODO:
      Formatter::formatSpeed(state.speed.avg, main.value, sizeof(main.value));
      main.unit = cfg.mainUnit;
      Formatter::formatDistance(state.distance.total, sub.value, sizeof(sub.value));
      sub.unit = cfg.subUnit;
      break;

    case Mode::MAX_CLK: {
      Formatter::formatSpeed(state.speed.max, main.value, sizeof(main.value));
      main.unit = cfg.mainUnit;
      int hour  = currentTime.hour;

      if (currentTime.year >= Config::Time::MIN_VALID_YEAR)
        hour = (hour + Config::Time::TIMEZONE_OFFSET_HOURS) % 24;
      Formatter::formatClock(hour, currentTime.minute, sub.value, sizeof(sub.value));
      sub.unit = cfg.subUnit;
      break;
    }
    }
  }

  bool operator==(const DisplayFrame &other) const {
    return (header == other.header) && (main == other.main) && (sub == other.sub);
  }
  bool operator!=(const DisplayFrame &other) const { return !(*this == other); }
};
