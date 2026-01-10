#pragma once

#include <Arduino.h>

#include "../Config.h"

enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    NONE
};

extern LogLevel currentLogLevel;

class Logger {
   public:
    static void setLogLevel(LogLevel level) {
        currentLogLevel = level;
    }

    static void log(LogLevel level, const char* file, int line, const char* func, const char* fmt, ...) __attribute__((format(printf, 5, 6)));
};

#ifdef DEBUG_MODE
    #define LOG_DEBUG(fmt, ...) Logger::log(LogLevel::DEBUG, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
    #define LOG_INFO(fmt, ...)  Logger::log(LogLevel::INFO,  __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
    #define LOG_WARN(fmt, ...)  Logger::log(LogLevel::WARN,  __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
    #define LOG_ERROR(fmt, ...) Logger::log(LogLevel::ERROR, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#else
    #define LOG_DEBUG(fmt, ...) ((void)0)
    #define LOG_INFO(fmt, ...)  ((void)0)
    #define LOG_WARN(fmt, ...)  ((void)0)
    #define LOG_ERROR(fmt, ...) ((void)0)
#endif
