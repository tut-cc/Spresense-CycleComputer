/*
 * ファイル: TripComputer.h
 * 説明: 距離、移動時間、最大速度、平均速度を計算する。
 */

#ifndef TRIPCOMPUTER_H
#define TRIPCOMPUTER_H

#include <Arduino.h>

class TripComputer {
 private:
  float maxSpeedKmh = 0.0f;
  double totalDistanceKm = 0.0;  // 長距離での精度のため double を使用 (AVR では
                                 // float と同じだが、Spresense は ARM)
  unsigned long movingTimeMs = 0;
  unsigned long elapsedTimeMs = 0;

  unsigned long lastUpdateMs = 0;
  const float moveThresholdKmh = 3.0f;  // 「移動中」とみなすための閾値

 public:
  TripComputer() {}

  void begin() {
    reset();
    lastUpdateMs = millis();
  }

  void update(float currentSpeedKmh, unsigned long currentMs) {
    if (lastUpdateMs == 0) {
      lastUpdateMs = currentMs;
      return;
    }

    unsigned long deltaMs = currentMs - lastUpdateMs;
    lastUpdateMs = currentMs;

    // 常に合計経過時間を更新
    elapsedTimeMs += deltaMs;

    // 最大速度を更新
    if (currentSpeedKmh > maxSpeedKmh) {
      maxSpeedKmh = currentSpeedKmh;
    }

    // 距離と時間の計算
    // 単純な積分: 距離 = 速度 * 時間
    // 速度は km/h、時間は ms。
    // 距離 (km) = 速度 (km/h) * (deltaMs / 1000.0 / 3600.0)

    if (currentSpeedKmh > moveThresholdKmh) {
      // 移動時間を加算
      movingTimeMs += deltaMs;

      // 距離を加算
      double hours = (double)deltaMs / 3600000.0;
      totalDistanceKm += (double)currentSpeedKmh * hours;
    }
  }

  void reset() {
    maxSpeedKmh = 0.0f;
    totalDistanceKm = 0.0;
    movingTimeMs = 0;
    elapsedTimeMs = 0;
    lastUpdateMs = millis();
  }

  float getMaxSpeedKmh() { return maxSpeedKmh; }

  float getDistanceKm() { return (float)totalDistanceKm; }

  // ms を文字列にフォーマットするヘルパー
  void msToTimeStr(unsigned long ms, char *buffer, size_t size) {
    unsigned long totalSeconds = ms / 1000;
    unsigned long hours = totalSeconds / 3600;
    unsigned long minutes = (totalSeconds % 3600) / 60;
    unsigned long seconds = totalSeconds % 60;

    snprintf(buffer, size, "%02lu:%02lu:%02lu", hours, minutes, seconds);
  }

  // 移動時間を "HH:MM:SS" 形式で返す
  void getMovingTimeStr(char *buffer, size_t size) {
    msToTimeStr(movingTimeMs, buffer, size);
  }

  // 合計経過時間を "HH:MM:SS" 形式で返す
  void getElapsedTimeStr(char *buffer, size_t size) {
    msToTimeStr(elapsedTimeMs, buffer, size);
  }

  float getAvgSpeedKmh() {
    if (movingTimeMs == 0) return 0.0f;

    double hours = (double)movingTimeMs / 3600000.0;
    return (float)(totalDistanceKm / hours);
  }
};

#endif
