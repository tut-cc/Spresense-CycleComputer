#pragma once

/**
 * @file Formatter.h
 * @brief 数値の文字列フォーマット関数群
 *
 * 速度、距離、時間などの数値を表示用の文字列に変換します。
 * 組み込み環境での高速動作のため、標準ライブラリ(sprintf等)を避け、
 * 独自の軽量実装を提供しています。
 */

#include <math.h>
#include <stddef.h>

namespace Formatter {
namespace Internal {

inline void reverse(char *begin, char *end) {
  char t;
  while (begin < end) {
    t        = *begin;
    *begin++ = *end;
    *end--   = t;
  }
}

inline char *itoa_impl(int value, char *str) {
  char *p = str;
  int   v = value;
  if (v < 0) v = -v;
  do {
    *p++ = (char)('0' + (v % 10));
    v /= 10;
  } while (v > 0);
  return p;
}

inline void itoa_pad(int value, char *str, int digits) {
  char *p = itoa_impl(value, str);
  while ((p - str) < digits) *p++ = '0';
  *p = '\0';
  reverse(str, p - 1);
}

inline float roundFloat(float value, int precision) {
  float mul = 1.0f;
  for (int i = 0; i < precision; ++i) mul *= 10.0f;
  float rounded = value;
  if (value >= 0.0f) rounded = floorf(value * mul + 0.5f) / mul;
  else rounded = ceilf(value * mul - 0.5f) / mul;
  return rounded;
}

inline char *writeIntPart(int value, char *buffer) {
  char *p = buffer;
  if (value == 0) {
    *p++ = '0';
  } else {
    while (value > 0) {
      *p++ = '0' + (value % 10);
      value /= 10;
    }
  }
  return p;
}

inline char *copyAndPad(char *dest, char *srcStart, char *srcEnd, int totalWidth) {
  int intLen  = (int)(srcEnd - srcStart);
  int padding = totalWidth - intLen;
  while (padding > 0) {
    *dest++ = ' ';
    padding--;
  }
  while (srcEnd > srcStart) *dest++ = *--srcEnd;
  return dest;
}

inline char *writeFracPart(float frac, int precision, char *dest) {
  int   intFrac = (int)(frac * powf(10.0f, precision) + 0.5f);
  char  temp[10];
  char *t = temp;
  for (int i = 0; i < precision; ++i) {
    *t++ = '0' + (intFrac % 10);
    intFrac /= 10;
  }
  while (t > temp) *dest++ = *--t;
  return dest;
}

} // namespace Internal

inline void ftoa_fixed(float value, char *buffer, int width, int precision) {
  if (value < 0) value = 0.0f;
  float mul = 1.0f;
  for (int i = 0; i < precision; ++i) mul *= 10.0f;
  float rounded  = floorf(value * mul + 0.5f);
  int   intPart  = (int)(rounded / mul);
  float rem      = rounded - (intPart * mul);
  float fracPart = rem / mul;
  char  tempInt[16];
  char *t            = Internal::writeIntPart(intPart, tempInt);
  int   contentWidth = (int)(t - tempInt);
  if (precision > 0) contentWidth += 1 + precision;
  char *p = buffer;
  while (contentWidth < width) {
    *p++ = ' ';
    contentWidth++;
  }
  while (t > tempInt) *p++ = *--t;
  if (precision > 0) {
    *p++ = '.';
    p    = Internal::writeFracPart(fracPart, precision, p);
  }
  *p = '\0';
}

inline void formatSpeed(float speedKmh, char *buffer, size_t size) {
  (void)size;
  ftoa_fixed(speedKmh, buffer, 4, 1);
}

inline void formatDistance(float distanceKm, char *buffer, size_t size) {
  (void)size;
  ftoa_fixed(distanceKm, buffer, 5, 2);
}

inline void formatDuration(unsigned long millis, char *buffer, size_t size) {
  (void)size;
  const unsigned long seconds = millis / 1000;
  const unsigned long h       = seconds / 3600;
  const unsigned long m       = (seconds % 3600) / 60;
  const unsigned long s       = seconds % 60;
  char               *p       = buffer;
  if (h > 0) {
    char  temp[10];
    char *t = Internal::itoa_impl((int)h, temp);
    while (t > temp) *p++ = *--t;
    *p++ = ':';
    Internal::itoa_pad((int)m, p, 2);
    p += 2;
  } else {
    Internal::itoa_pad((int)m, p, 2);
    p += 2;
  }
  *p++ = ':';
  Internal::itoa_pad((int)s, p, 2);
  p += 2;
  *p = '\0';
}

inline void formatClock(int h, int m, char *buffer) {
  char *p = buffer;
  Internal::itoa_pad(h, p, 2);
  p += 2;
  *p++ = ':';
  Internal::itoa_pad(m, p, 2);
  p += 2;
  *p = '\0';
}

} // namespace Formatter
