#pragma once

#include <Arduino.h>

// プラットフォームに依存しない方法でfloatを文字列にフォーマットするヘルパー関数
inline void formatFloat(float val, int width, int prec, char* buf, size_t size) {
#ifdef ARDUINO_ARCH_SPRESENSE
    // Spresense (ARM, 標準 C) -> snprintf を使用
    char fmt[6];
    snprintf(fmt, sizeof(fmt), "%%%d.%df", width, prec);
    snprintf(buf, size, fmt, val);
#else
    // Arduino (AVR) -> snprintf %f はサポートされていないため、dtostrf を使用
    dtostrf(val, width, prec, buf);
    (void)size;  // unused parameter suppression
#endif
}
