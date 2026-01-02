/*
 * ファイル: Utils.h
 * 説明: プラットフォーム対話ユーティリティとヘルパー関数。
 *       文字列フォーマットのクロスプラットフォーム互換性を提供する。
 */

#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

#include "../Config.h"

// プラットフォームに依存しない方法でfloatを文字列にフォーマットするヘルパー関数
inline void formatFloat(float val, int width, int prec, char* buf,
                        size_t size) {
#ifdef IS_SPRESENSE
  // Spresense (ARM, 標準 C) -> snprintf を使用
  char fmt[6];
  snprintf(fmt, sizeof(fmt), "%%%d.%df", width, prec);
  snprintf(buf, size, fmt, val);
#else
  // Arduino (AVR) -> snprintf %f はサポートされていないため、dtostrf を使用
  dtostrf(val, width, prec, buf);
#endif
}

#endif
