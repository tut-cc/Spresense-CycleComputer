#include "Logger.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>

#ifdef DEBUG_MODE
LogLevel currentLogLevel = LogLevel::INFO;
#else
LogLevel currentLogLevel = LogLevel::NONE;
#endif

#ifdef DEBUG_MODE
void Logger::log(LogLevel level, const char* file, int line, const char* func, const char* fmt, ...) {
    if (level < currentLogLevel) return;

    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    const char* levelStr = "";
    switch (level) {
        case LogLevel::DEBUG:
            levelStr = "DEBUG";
            break;
        case LogLevel::INFO:
            levelStr = "INFO";
            break;
        case LogLevel::WARN:
            levelStr = "WARN";
            break;
        case LogLevel::ERROR:
            levelStr = "ERROR";
            break;
        default:
            break;
    }

    // Timestamp
    unsigned long t = millis();
    unsigned long h = t / 3600000;
    t %= 3600000;
    unsigned long m = t / 60000;
    t %= 60000;
    unsigned long s = t / 1000;
    unsigned long ms = t % 1000;

    char timestamp[32];
    snprintf(timestamp, sizeof(timestamp), "[%02lu:%02lu:%02lu.%03lu]", h, m, s, ms);

    // File name (basename)
    const char* filename = strrchr(file, '/');
    if (filename)
        filename++;
    else
        filename = file;

    Serial.print(timestamp);
    Serial.print(" [");
    Serial.print(levelStr);
    Serial.print("] ");
    Serial.print(filename);
    Serial.print(":");
    Serial.print(line);
    Serial.print(" (");
    Serial.print(func);
    Serial.print("): ");
    Serial.println(buf);
}
#endif
