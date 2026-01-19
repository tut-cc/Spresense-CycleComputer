#pragma once
/**
 * @file Button.h
 * @brief 物理ボタンの入力処理（デバウンス付き）
 *
 * チャタリング（接点バウンス）を除去して、
 * 安定したボタン状態を取得できます。
 */

#include <Arduino.h>

constexpr unsigned long DEBOUNCE_DELAY_MS = 50; ///< デバウンス時間(ms)

/**
 * @class Button
 * @brief 1つの物理ボタンを管理
 *
 * 状態遷移図:
 * High ←→ WaitStabilizeLow ←→ Low ←→ WaitStabilizeHigh ←→ High
 */
class Button {
public:
  /**
   * @brief ボタンの状態
   */
  enum class State {
    High,             ///< ボタンが離されている
    WaitStablizeHigh, ///< 離されたか確認中（デバウンス）
    Low,              ///< ボタンが押されている
    WaitStablizeLow   ///< 押されたか確認中（デバウンス）
  };

private:
  const int     pinNumber;           ///< GPIOピン番号
  State         state;               ///< 現在の状態
  unsigned long lastStateChangeTime; ///< 最後に状態が変わった時刻
  bool          pressEdge;           ///< 押された瞬間フラグ

public:
  Button(int pin) : pinNumber(pin), state(State::High), pressEdge(false) {}

  /** @brief 初期化（プルアップ設定） */
  void begin() {
    pinMode(pinNumber, INPUT_PULLUP);
    state     = (digitalRead(pinNumber) == LOW) ? State::Low : State::High;
    pressEdge = false;
  }

  /**
   * @brief ボタン状態を更新
   * @note 毎ループ呼び出してください
   */
  void update() {
    pressEdge                       = false;
    const bool          rawPinLevel = digitalRead(pinNumber);
    const unsigned long now         = millis();

    switch (state) {
    case State::High: // 押されていない状態
      if (rawPinLevel == LOW) changeState(State::WaitStablizeLow, now);
      break;

    case State::WaitStablizeLow: // 押された可能性を確認中
      if (rawPinLevel == HIGH) changeState(State::High, now);
      else if (now - lastStateChangeTime > DEBOUNCE_DELAY_MS) {
        changeState(State::Low, now);
        pressEdge = true; // ★押された瞬間
      }
      break;

    case State::Low: // 押されている状態
      if (rawPinLevel == HIGH) changeState(State::WaitStablizeHigh, now);
      break;

    case State::WaitStablizeHigh: // 離された可能性を確認中
      if (rawPinLevel == LOW) changeState(State::Low, now);
      else if (now - lastStateChangeTime > DEBOUNCE_DELAY_MS) changeState(State::High, now);
      break;
    }
  }

  /** @brief ボタンが押された瞬間か（エッジ検出） */
  bool isPressed() const { return pressEdge; }

  /** @brief ボタンが押され続けているか */
  bool isHeld() const { return (state == State::Low || state == State::WaitStablizeHigh); }

private:
  void changeState(State newState, unsigned long now) {
    state               = newState;
    lastStateChangeTime = now;
  }
};
