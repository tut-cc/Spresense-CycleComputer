#pragma once

#include <Arduino.h>

inline void formatFloat(float val, int width, int prec, char* buf, size_t size) {
    char fmt[6];
    snprintf(fmt, sizeof(fmt), "%%%d.%df", width, prec);
    snprintf(buf, size, fmt, val);
}
