#pragma once

#include "../DataStructures.h"
#include "Renderer.h"
#include <Arduino.h>

class Mode {
public:
  enum class ID { SPD_TIM, AVG_ODO, MAX_CLK };

  // DisplayDataからUI表示用のDisplayFrameを生成する
  static void fillFrame(DisplayFrame &frame, const DisplayData &data) {
    // ヘッダー
    strcpy(frame.header.modeSpeed, data.modeSpeedLabel);
    strcpy(frame.header.modeTime, data.modeTimeLabel);

    // メイン数値（速度など）
    Formatter::formatSpeed(data.mainValue, frame.main.value, sizeof(frame.main.value));
    strcpy(frame.main.unit, data.mainUnit);

    // サブ数値（時間、距離、時計）
    if (data.shouldBlink) {
      strcpy(frame.sub.value, "");
    } else {
      switch (data.subType) {
      case DisplayData::SubType::Duration:
        Formatter::formatDuration(data.subValue.durationMs, frame.sub.value,
                                  sizeof(frame.sub.value));
        break;
      case DisplayData::SubType::Distance:
        Formatter::formatDistance(data.subValue.distanceKm, frame.sub.value,
                                  sizeof(frame.sub.value));
        break;
      case DisplayData::SubType::Clock:
        // Clock構造体は使わず値を直接渡すか、一時的に構築
        snprintf(frame.sub.value, sizeof(frame.sub.value), "%02d:%02d",
                 data.subValue.clockTime.hour, data.subValue.clockTime.minute);
        break;
      }
    }
    strcpy(frame.sub.unit, data.subUnit);
  }
};
