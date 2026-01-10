#pragma once

#include <Arduino.h>

class TripComputer {
   private:
    float maxSpeedKmh = 0.0f;
    double totalDistanceKm = 0.0;  // 長距離での精度のため double を使用
    unsigned long movingTimeMs = 0;
    unsigned long elapsedTimeMs = 0;
    unsigned long lastUpdateMs = 0;

   public:
    void begin();
    void update(float currentSpeedKmh, unsigned long currentMs);
    void reset();
    float getMaxSpeedKmh();
    float getDistanceKm();
    void getMovingTimeStr(char *buffer, size_t size);
    void getElapsedTimeStr(char *buffer, size_t size);
    float getAvgSpeedKmh();
    void msToTimeStr(unsigned long ms, char *buffer, size_t size);
};
