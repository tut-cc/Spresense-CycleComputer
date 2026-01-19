#pragma once
/**
 * @file Input.h
 * @brief 2ボタン入力の処理クラス
 *
 * SELECT(モード切替)とPAUSE(一時停止)の2つのボタンを管理します。
 * 同時押しでリセット、長押しで全データリセットを検出します。
 *
 * 入力パターン:
 * - SELECTボタン単押し → モード切替
 * - PAUSEボタン単押し → 一時停止トグル
 * - 2ボタン同時短押し → トリップ等リセット
 * - 2ボタン同時長押し(3秒) → 全データリセット
 */

#include "../hardware/Button.h"

constexpr unsigned long SINGLE_PRESS_MS = 50;   ///< 単押し確定時間(ms)
constexpr unsigned long LONG_PRESS_MS   = 3000; ///< 長押し判定時間(ms)

/**
 * @class Input
 * @brief 2ボタン入力の状態管理
 */
class Input {
public:
  /**
   * @brief 入力イベント
   */
  enum class Event {
    NONE,      ///< イベントなし
    SELECT,    ///< モード切替
    PAUSE,     ///< 一時停止トグル
    RESET,     ///< リセット（モードに応じた）
    RESET_LONG ///< 全データリセット
  };

private:
  /**
   * @brief 入力判定の状態
   */
  enum class State {
    Idle,             ///< 何も押されていない
    MayBeSingle,      ///< 単ボタン押し確認中
    MayBeDoubleShort, ///< 2ボタン同時押し確認中
    MustBeDoubleLong  ///< 2ボタン長押し確定
  };

  Button selectButton; ///< SELECTボタン
  Button pauseButton;  ///< PAUSEボタン

  State state                = State::Idle;
  Event potentialSingleEvent = Event::NONE; ///< 単押し候補イベント

  unsigned long stateEnterTime = 0; ///< 現在の状態に入った時刻

public:
  Input(int selectButtonPin, int pauseButtonPin)
      : selectButton(selectButtonPin), pauseButton(pauseButtonPin) {}

  void begin() {
    selectButton.begin();
    pauseButton.begin();
  }

  /**
   * @brief 入力状態を更新し、イベントを返す
   * @return 発生したイベント
   * @note 毎ループ呼び出してください
   */
  Event update() {
    selectButton.update();
    pauseButton.update();

    const bool          selectPressed = selectButton.isPressed();
    const bool          selectHeld    = selectButton.isHeld();
    const bool          pausePressed  = pauseButton.isPressed();
    const bool          pauseHeld     = pauseButton.isHeld();
    const unsigned long now           = millis();

    switch (state) {
    case State::Idle: // 待機状態
      // 両ボタン同時押し
      if (selectPressed && pausePressed) {
        changeState(State::MayBeDoubleShort, now);
        return Event::NONE;
      }
      // SELECTのみ押した
      if (selectPressed) {
        potentialSingleEvent = Event::SELECT;
        changeState(State::MayBeSingle, now);
        return Event::NONE;
      }
      // PAUSEのみ押した
      if (pausePressed) {
        potentialSingleEvent = Event::PAUSE;
        changeState(State::MayBeSingle, now);
        return Event::NONE;
      }
      break;

    case State::MayBeSingle: // 単押し確認中
      // もう一方のボタンも押された → 同時押しへ遷移
      if ((potentialSingleEvent == Event::SELECT && pausePressed) ||
          (potentialSingleEvent == Event::PAUSE && selectPressed)) {
        changeState(State::MayBeDoubleShort, now);
        return Event::NONE;
      }
      // デバウンス時間経過 → 単押し確定
      if (now - stateEnterTime > SINGLE_PRESS_MS) {
        changeState(State::Idle, now);
        return potentialSingleEvent;
      }
      break;

    case State::MayBeDoubleShort: // 2ボタン同時押し確認中
      // どちらかが離された → 短押しリセット確定
      if (!selectHeld || !pauseHeld) {
        changeState(State::Idle, now);
        return Event::RESET;
      }
      // 長押し時間経過 → 全リセット確定
      if (now - stateEnterTime > LONG_PRESS_MS) {
        changeState(State::MustBeDoubleLong, now);
        return Event::RESET_LONG;
      }
      break;

    case State::MustBeDoubleLong: // 長押し確定後
      // 両方離されるまで待機
      if (!selectHeld && !pauseHeld) changeState(State::Idle, now);
      break;
    }

    return Event::NONE;
  }

private:
  void changeState(State newState, unsigned long now) {
    state          = newState;
    stateEnterTime = now;
  }
};
