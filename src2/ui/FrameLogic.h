#pragma once
/**
 * @file FrameLogic.h
 * @brief 表示フレームの構築ロジック
 *
 * DisplayStateからDisplayFrame（描画用データ）を生成します。
 * Formatterを使って数値を文字列に変換し、フレームを構築します。
 */

#include "../common/DataStructures.h"
#include "../common/Formatter.h"
#include <string.h>

namespace FrameLogic {

/**
 * @brief 内部実装用名前空間
 */
namespace Internal {
using FormatterFunc = void (*)(const DisplayState &, char *, size_t);

/** @brief 経過時間をフォーマット */
inline void fmtDuration(const DisplayState &d, char *b, size_t s) {
  Formatter::formatDuration(d.subValue.durationMs, b, s);
}

/** @brief 距離をフォーマット */
inline void fmtDistance(const DisplayState &d, char *b, size_t s) {
  Formatter::formatDistance(d.subValue.distanceKm, b, s);
}

/** @brief 時刻をフォーマット */
inline void fmtClock(const DisplayState &d, char *b, size_t s) {
  (void)s;
  Formatter::formatClock(d.subValue.clockTime.hour, d.subValue.clockTime.minute, b);
}
} // namespace Internal

/**
 * @brief 表示フレームを構築
 * @param data 表示用データ
 * @return DisplayFrame 描画用フレーム
 *
 * 点滅中(shouldBlink==true)はサブ表示を空にします。
 */
inline DisplayFrame buildFrame(const DisplayState &data) {
  DisplayFrame frame;

  // ヘッダー: GPS状態ラベル
  static const char *FIX_LABELS[] = {"WAIT", "2D", "3D"};
  int                fixIdx       = (int)data.fixMode;
  if (fixIdx < 0 || fixIdx > 2) fixIdx = 0;
  frame.header.fixStatus = FIX_LABELS[fixIdx];

  // ヘッダー: モードラベル
  frame.header.modeSpeed = data.modeSpeedLabel;
  frame.header.modeTime  = data.modeTimeLabel;

  // メイン表示: 速度
  Formatter::formatSpeed(data.mainValue, frame.main.value, sizeof(frame.main.value));
  frame.main.unit = data.mainUnit;

  // サブ表示: 点滅中は空、そうでなければモードに応じた値
  if (data.shouldBlink) {
    strcpy(frame.sub.value, "");
    frame.sub.unit = "";
  } else {
    // サブタイプに応じたフォーマッタを使用
    static const Internal::FormatterFunc formatters[] = {Internal::fmtDuration,
                                                         Internal::fmtDistance, Internal::fmtClock};
    formatters[(int)data.subType](data, frame.sub.value, sizeof(frame.sub.value));
    frame.sub.unit = data.subUnit;
  }
  return frame;
}

} // namespace FrameLogic
