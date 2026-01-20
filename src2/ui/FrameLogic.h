#pragma once

/**
 * @file FrameLogic.h
 * @brief 表示フレームの構築ロジック
 *
 * トリップ状態とGNSS データから、OLEDに表示する
 * DisplayFrame構造体を生成します。
 */

#include "../common/Config.h"
#include "../common/DataStructures.h"
#include "../common/Formatter.h"
#include <Arduino.h>
#include <string.h>

namespace FrameLogic {

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

inline DisplayFrame buildFrame(const TripStateBase &state, const GnssData &gnss,
                               const SpGnssTime &currentTime, Mode mode) {
  DisplayFrame frame;

  const ModeConfig &cfg = CONFIGS[(int)mode];

  const SpFixMode fixMode = (SpFixMode)gnss.navData.posFixMode;
  if (fixMode == Fix3D) frame.header.fixStatus = FIX_LABELS[2];
  else if (fixMode == Fix2D) frame.header.fixStatus = FIX_LABELS[1];
  else frame.header.fixStatus = FIX_LABELS[0];
  frame.header.modeSpeed = cfg.speedLabel;
  frame.header.modeTime  = cfg.timeLabel;

  const bool isBlinkPhase = state.isPaused() && ((millis() / 500) % 2 == 0);
  const bool shouldBlink  = (mode == Mode::SPD_TIM) && isBlinkPhase;

  switch (mode) {
  case Mode::SPD_TIM:
    Formatter::formatSpeed(state.speed.current, frame.main.value, sizeof(frame.main.value));
    frame.main.unit = cfg.mainUnit;
    if (shouldBlink) {
      strcpy(frame.sub.value, "");
      frame.sub.unit = "";
    } else {
      Formatter::formatDuration(state.time.elapsed, frame.sub.value, sizeof(frame.sub.value));
      frame.sub.unit = cfg.subUnit;
    }
    break;

  case Mode::AVG_ODO:
    Formatter::formatSpeed(state.speed.avg, frame.main.value, sizeof(frame.main.value));
    frame.main.unit = cfg.mainUnit;
    Formatter::formatDistance(state.distance.total, frame.sub.value, sizeof(frame.sub.value));
    frame.sub.unit = cfg.subUnit;
    break;

  case Mode::MAX_CLK: {
    Formatter::formatSpeed(state.speed.max, frame.main.value, sizeof(frame.main.value));
    frame.main.unit = cfg.mainUnit;
    int hour        = currentTime.hour;
    // GPS時刻(UTC)をJST(+9時間)に変換
    if (currentTime.year >= Config::Time::MIN_VALID_YEAR)
      hour = (hour + Config::Time::TIMEZONE_OFFSET_HOURS) % 24;
    Formatter::formatClock(hour, currentTime.minute, frame.sub.value);
    frame.sub.unit = cfg.subUnit;
    break;
  }
  }

  return frame;
}

} // namespace FrameLogic
