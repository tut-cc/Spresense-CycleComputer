#pragma once

#include <stddef.h>

namespace Formatter {

inline void reverse(char *begin, char *end) {
  char t;
  while (begin < end) {
    t        = *begin;
    *begin++ = *end;
    *end--   = t;
  }
}

inline void itoa_pad(int value, char *str, int digits) {
  char *p = str;
  int   v = value;
  if (v < 0) v = -v; // handle unsigned only for time

  do {
    *p++ = (char)('0' + (v % 10));
    v /= 10;
  } while (v > 0);

  while ((p - str) < digits) *p++ = '0';
  *p = '\0';
  reverse(str, p - 1);
}

inline void ftoa_fixed(float value, char *buffer, int width, int precision) {
  if (value < 0) value = 0; // Negative speed/distance unlikely

  int   int_part = (int)value;
  float rem      = value - int_part;

  // Rounding
  float mul = 1.0f;
  for (int i = 0; i < precision; ++i) mul *= 10.0f;
  rem = rem * mul + 0.5f;
  if (rem >= mul) {
    rem -= mul;
    int_part++;
  }
  int frac_part = (int)rem;

  char *p = buffer;

  // Convert integer part
  char  temp[16];
  char *t = temp;
  int   v = int_part;
  if (v == 0) *t++ = '0';
  else {
    while (v > 0) {
      *t++ = '0' + (v % 10);
      v /= 10;
    }
  }

  // Padding
  int len = (t - temp) + 1 + precision; // int_len + dot + precision
  while (len < width) {
    *p++ = ' ';
    len++;
  }

  while (t > temp) *p++ = *--t;

  if (precision > 0) {
    *p++ = '.';
    // frac part padding (e.g. 05)
    t = temp;
    for (int i = 0; i < precision; ++i) {
      *t++ = '0' + (frac_part % 10);
      frac_part /= 10;
    }
    while (t > temp) *p++ = *--t;
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

  char *p = buffer;

  if (h > 0) {
    int   hv = (int)h;
    char  temp[10];
    char *t = temp;
    do {
      *t++ = '0' + (hv % 10);
      hv /= 10;
    } while (hv);
    while (t > temp) *p++ = *--t;
    *p++ = ':';

    itoa_pad((int)m, p, 2);
    p += 2;
    *p++ = ':';
    itoa_pad((int)s, p, 2);
    p += 2;
    *p = '\0';
  } else {
    itoa_pad((int)m, p, 2);
    p += 2;
    *p++ = ':';
    itoa_pad((int)s, p, 2);
    p += 2;
    *p = '\0';
  }
}

inline void formatClock(int h, int m, char *buffer) {
  char *p = buffer;
  itoa_pad(h, p, 2);
  p += 2;
  *p++ = ':';
  itoa_pad(m, p, 2);
  p += 2;
  *p = '\0';
}

} // namespace Formatter
