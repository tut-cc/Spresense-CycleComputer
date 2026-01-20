#pragma once

/**
 * @file Formatter.h
 * @brief 数値の文字列フォーマット関数群
 *
 * 速度、距離、時間などの数値を表示用の文字列に変換します。
 * 組み込み環境での安全な動作のため、境界チェックを徹底しています。
 */

#include <math.h>
#include <stddef.h>
#include <string.h>

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

inline char *itoa_impl(int value, char *str, char *limit) {
  char *p = str;
  int   v = value;
  if (v < 0) v = -v;
  do {
    if (p >= limit) break;
    *p++ = (char)('0' + (v % 10));
    v /= 10;
  } while (v > 0);
  return p;
}

inline void itoa_pad(int value, char *str, size_t size, int digits) {
  if (size == 0) return;
  char *limit = str + size - 1;
  char *p     = itoa_impl(value, str, limit);
  while ((p - str) < digits && p < limit) *p++ = '0';
  *p = '\0';
  reverse(str, p - 1);
}

} // namespace Internal

inline void ftoa_fixed(float value, char *buffer, size_t size, int width, int precision) {
  if (size == 0) return;
  if (value < 0) value = 0.0f;

  float mul = 1.0f;
  for (int i = 0; i < precision; ++i) mul *= 10.0f;
  float rounded = floorf(value * mul + 0.5f);
  int   intPart = (int)(rounded / mul);
  int   fracInt = (int)(rounded) % (int)mul;

  char  tempInt[16];
  char *limitInt = tempInt + sizeof(tempInt) - 1;
  char *t        = Internal::itoa_impl(intPart, tempInt, limitInt);
  int   intLen   = (int)(t - tempInt);

  int contentWidth = intLen;
  if (precision > 0) contentWidth += 1 + precision;

  char *p     = buffer;
  char *limit = buffer + size - 1;

  // Padding
  while (contentWidth < width && p < limit) {
    *p++ = ' ';
    contentWidth++;
  }

  // Integer part
  while (t > tempInt && p < limit) *p++ = *--t;

  // Decimal part
  if (precision > 0 && p < limit) {
    *p++ = '.';
    char  tempFrac[10];
    char *fLimit = tempFrac + sizeof(tempFrac) - 1;
    char *f      = Internal::itoa_impl(fracInt, tempFrac, fLimit);
    int   fLen   = (int)(f - tempFrac);
    while (fLen < precision && p < limit) {
      *p++ = '0';
      fLen++;
    }
    while (f > tempFrac && p < limit) *p++ = *--f;
  }
  *p = '\0';
}

inline void formatSpeed(float speedKmh, char *buffer, size_t size) {
  ftoa_fixed(speedKmh, buffer, size, 4, 1);
}

inline void formatDistance(float distanceKm, char *buffer, size_t size) {
  ftoa_fixed(distanceKm, buffer, size, 5, 2);
}

inline void formatDuration(unsigned long millis, char *buffer, size_t size) {
  if (size == 0) return;
  const unsigned long seconds = millis / 1000;
  const unsigned long h       = seconds / 3600;
  const unsigned long m       = (seconds % 3600) / 60;
  const unsigned long s       = seconds % 60;

  char *p     = buffer;
  char *limit = buffer + size - 1;

  if (h > 0) {
    char  temp[16];
    char *t = Internal::itoa_impl((int)h, temp, temp + sizeof(temp) - 1);
    while (t > temp && p < limit) *p++ = *--t;
    if (p < limit) *p++ = ':';
  }

  if (p + 2 <= limit) {
    Internal::itoa_pad((int)m, p, (size_t)(limit - p + 1), 2);
    p += 2;
  }
  if (p < limit) *p++ = ':';
  if (p + 2 <= limit) {
    Internal::itoa_pad((int)s, p, (size_t)(limit - p + 1), 2);
    p += 2;
  }
  *p = '\0';
}

inline void formatClock(int h, int m, char *buffer, size_t size) {
  if (size < 6) return; // "HH:MM\0"
  char *p = buffer;
  Internal::itoa_pad(h, p, size, 2);
  p += 2;
  *p++ = ':';
  Internal::itoa_pad(m, p, size - 3, 2);
  p += 2;
  *p = '\0';
}

} // namespace Formatter
