#pragma once
/**
 * @file DisplayLogic.h
 * @brief 表示用データを生成するロジック
 *
 * TripState（走行状態）からDisplayState（表示用データ）を生成します。
 * 表示モードに応じて適切なラベルと値を選択します。
 *
 * 表示モード:
 * - SPD_TIM: 現在速度 + 経過時間
 * - AVG_ODO: 平均速度 + 総走行距離
 * - MAX_CLK: 最高速度 + 現在時刻
 */

#include "../common/DataStructures.h"
#include <Arduino.h>

namespace DisplayLogic {

/**
 * @brief 走行状態から表示用データを生成
 * @param state 走行状態
 * @param gnss GNSSデータ
 * @param currentTime 現在時刻（RTC）
 * @param mode 表示モード
 * @return DisplayState 表示用データ
 *
 * 点滅制御：
 * - SPD_TIMモードで一時停止中の場合、500ms間隔で点滅します
 * - 点滅中はshouldBlinkがtrueになり、サブ表示が消えます
 */
inline DisplayState create(const TripStateBase &state, const GnssData &gnss,
                           const SpGnssTime &currentTime, Mode mode) {
  DisplayState data;

  // GPS受信状態をコピー
  data.fixMode = (SpFixMode)gnss.navData.posFixMode;

  // 点滅判定: SPD_TIMモードでポーズ中、かつ500ms間隔で切り替え
  const bool isBlinkPhase = state.isPaused() && ((millis() / 500) % 2 == 0);
  data.shouldBlink        = (mode == Mode::SPD_TIM) && isBlinkPhase;
  data.updateStatus       = state.updateStatus;

  /**
   * @brief モードごとの設定
   *
   * 各モードで使用するラベル・単位・サブ表示タイプを定義しています。
   */
  struct ModeConfig {
    const char           *speedLabel; ///< 速度欄のラベル
    const char           *timeLabel;  ///< 時間欄のラベル
    const char           *mainUnit;   ///< メイン値の単位
    const char           *subUnit;    ///< サブ値の単位
    DisplayState::SubType subType;    ///< サブ表示の種類
  };

  // 各モードの設定をテーブル化（switch文より効率的）
  static const ModeConfig CONFIGS[] = {
      {"SPD", "Time", "km/h", "", DisplayState::SubType::Duration},  // SPD_TIM
      {"AVG", "Odo", "km/h", "km", DisplayState::SubType::Distance}, // AVG_ODO
      {"MAX", "Clock", "km/h", "", DisplayState::SubType::Clock}     // MAX_CLK
  };

  // モード番号（0,1,2）でインデックス
  const ModeConfig &cfg = CONFIGS[(int)mode];

  // 共通設定をコピー
  data.modeSpeedLabel = cfg.speedLabel;
  data.modeTimeLabel  = cfg.timeLabel;
  data.mainValue      = 0.0f; // デフォルト初期化
  data.mainUnit       = cfg.mainUnit;
  data.subType        = cfg.subType;
  data.subUnit        = cfg.subUnit;

  // モードごとに異なる値を設定
  switch (mode) {
  case Mode::SPD_TIM:
    // 現在速度 + 経過時間
    data.mainValue           = state.speed.current;
    data.subValue.durationMs = state.time.elapsed;
    break;

  case Mode::AVG_ODO:
    // 平均速度 + 総走行距離
    data.mainValue           = state.speed.avg;
    data.subValue.distanceKm = state.distance.total;
    break;

  case Mode::MAX_CLK:
    // 最高速度 + 現在時刻
    data.mainValue = state.speed.max;

    // 時刻はUTCからJSTに変換（+9時間）
    // 年が2026以降の場合のみ変換（GPS同期済みを示す）
    int hour = currentTime.hour;
    if (currentTime.year >= 2026) hour = (hour + 9) % 24;
    data.subValue.clockTime.hour   = hour;
    data.subValue.clockTime.minute = currentTime.minute;
    break;
  }

  return data;
}

} // namespace DisplayLogic
