#pragma once

#include <Arduino.h>
#include "../Config.h"

class Logger {
public:
    template <typename T>
    static void log(const T& msg) {
#ifdef DEBUG_MODE
        Serial.print(msg);
#endif
    }

    template <typename T>
    static void logln(const T& msg) {
#ifdef DEBUG_MODE
        Serial.println(msg);
#endif
    }
};
