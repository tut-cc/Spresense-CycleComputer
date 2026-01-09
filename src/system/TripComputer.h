#pragma once

#include <Arduino.h>

class TripComputer {
   private:
    float maxSpeedKmh = 0.0f;
    double totalDistanceKm = 0.0;  // 長距離での精度のため double を使用 (AVR では
                                   // float と同じだが、Spresense は ARM)
    unsigned long movingTimeMs = 0;
    unsigned long elapsedTimeMs = 0;

    unsigned long lastUpdateMs = 0;

   public:
    TripComputer();

    void begin();
    void update(float currentSpeedKmh, unsigned long currentMs);
    void reset();
    float getMaxSpeedKmh();
    float getDistanceKm();

    // ms を文字列にフォーマットするヘルパー
    void msToTimeStr(unsigned long ms, char *buffer, size_t size);

    // 移動時間を "HH:MM:SS" 形式で返す
    void getMovingTimeStr(char *buffer, size_t size);

    // 合計経過時間を "HH:MM:SS" 形式で返す
    void getElapsedTimeStr(char *buffer, size_t size);

    float getAvgSpeedKmh();
};
