#pragma once
/**
 * @file InputLogic.h
 * @brief ユーザー入力（ボタン操作）のロジック処理
 *
 * ボタンイベントに応じて、モード切替やリセット処理を行います。
 */

#include "../common/DataStructures.h"
#include "../ui/Input.h"

namespace InputLogic {

/**
 * @brief リセットの種類
 */
enum class ResetType {
  None,          // リセットなし
  Trip,          // トリップのみリセット
  MaxSpeed,      // 最高速度のみリセット
  All,           // 全データリセット（EEPROM保持）
  AllWithStorage // 全データ＋EEPROMクリア
};

/**
 * @brief リセット種類を決定
 * @param event イベント
 * @param currentMode 現在のモード
 * @return リセット種類
 *
 * 長押し→全リセット、短押し→モードに応じたリセット
 */
inline ResetType determineResetType(Input::Event event, Mode currentMode) {
  if (event == Input::Event::RESET_LONG) { return ResetType::AllWithStorage; }

  if (event == Input::Event::RESET) {
    static const ResetType RESET_MAP[] = {
        ResetType::Trip,     // SPD_TIM: トリップリセット
        ResetType::All,      // AVG_ODO: 全リセット
        ResetType::MaxSpeed, // MAX_CLK: 最高速度リセット
    };
    return RESET_MAP[(int)currentMode];
  }

  return ResetType::None;
}

/**
 * @brief リセットを適用
 */
template <typename T> inline void applyReset(T &state, ResetType resetType) {
  switch (resetType) {
  case ResetType::Trip:
    state.resetTrip();
    break;
  case ResetType::MaxSpeed:
    state.resetMaxSpeed();
    break;
  case ResetType::All:
  case ResetType::AllWithStorage:
    state.resetAll();
    break;
  default:
    break;
  }
}

/**
 * @brief 一時停止を切り替え
 */
inline void applyPause(TripStateBase &state) {
  state.status = (state.status == TripStateBase::Status::Paused) ? TripStateBase::Status::Stopped
                                                                 : TripStateBase::Status::Paused;
  state.forceUpdate();
}

/**
 * @brief モード切替
 */
inline Mode switchMode(Mode currentMode, Input::Event event) {
  if (event == Input::Event::SELECT) {
    return static_cast<Mode>((static_cast<int>(currentMode) + 1) % 3);
  }
  return currentMode;
}

/**
 * @brief イベント処理の結果
 */
struct UserInputResult {
  Mode newMode;            ///< 新しいモード
  bool shouldClearStorage; ///< EEPROMクリアが必要か
};

/**
 * @brief イベントを処理
 * @return 処理結果
 */
template <typename T>
inline UserInputResult handleEvent(T &state, Mode currentMode, Input::Event event) {
  UserInputResult result = {currentMode, false};
  if (event == Input::Event::NONE) return result;

  result.newMode = switchMode(currentMode, event);
  if (result.newMode != currentMode) state.forceUpdate();

  switch (event) {
  case Input::Event::PAUSE:
    applyPause(state);
    break;
  case Input::Event::RESET:
  case Input::Event::RESET_LONG: {
    ResetType r = determineResetType(event, currentMode);
    applyReset(state, r);
    result.shouldClearStorage = (r == ResetType::AllWithStorage);
    break;
  }
  default:
    break;
  }

  return result;
}

} // namespace InputLogic
