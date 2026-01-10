#include "TripComputer.h"
#include "../Config.h"

void TripComputer::begin() {
    reset();
    lastUpdateMs = millis();
}

void TripComputer::update(float currentSpeedKmh, unsigned long currentMs) {
    if (lastUpdateMs == 0) {
        lastUpdateMs = currentMs;
        return;
    }

    unsigned long deltaMs = currentMs - lastUpdateMs;
    lastUpdateMs = currentMs;

    elapsedTimeMs += deltaMs; // 常に合計経過時間を更新

    if (currentSpeedKmh > maxSpeedKmh) maxSpeedKmh = currentSpeedKmh;

    if (currentSpeedKmh > Config::Trip::MOVE_THRESHOLD_KMH) {
        movingTimeMs += deltaMs; // 移動時間を加算
        double hours = (double)deltaMs / 3600000.0;
        totalDistanceKm += (double)currentSpeedKmh * hours;
    }
}

void TripComputer::reset() {
    maxSpeedKmh = 0.0f;
    totalDistanceKm = 0.0;
    movingTimeMs = 0;
    elapsedTimeMs = 0;
    lastUpdateMs = millis();
}

float TripComputer::getMaxSpeedKmh() {
    return maxSpeedKmh;
}

float TripComputer::getDistanceKm() {
    return (float)totalDistanceKm;
}

void TripComputer::msToTimeStr(unsigned long ms, char *buffer, size_t size) {
    unsigned long totalSeconds = ms / 1000;
    unsigned long hours = totalSeconds / 3600;
    unsigned long minutes = (totalSeconds % 3600) / 60;
    unsigned long seconds = totalSeconds % 60;

    snprintf(buffer, size, "%02lu:%02lu:%02lu", hours, minutes, seconds);
}

void TripComputer::getMovingTimeStr(char *buffer, size_t size) {
    msToTimeStr(movingTimeMs, buffer, size);
}

void TripComputer::getElapsedTimeStr(char *buffer, size_t size) {
    msToTimeStr(elapsedTimeMs, buffer, size);
}

float TripComputer::getAvgSpeedKmh() {
    if (movingTimeMs == 0) return 0.0f;

    double hours = (double)movingTimeMs / 3600000.0;
    return (float)(totalDistanceKm / hours);
}
