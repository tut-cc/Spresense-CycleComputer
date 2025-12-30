/*
 * ファイル: TripComputer.h
 * 説明: 距離、移動時間、最大速度、平均速度を計算する。
 */

#ifndef TRIPCOMPUTER_H
#define TRIPCOMPUTER_H

#include <Arduino.h>

class TripComputer {
   private:
    float _maxSpeedKmh = 0.0f;
    double _totalDistanceKm =
        0.0;  // 長距離での精度のため double を使用 (AVR では float と同じだが、Spresense は ARM)
    unsigned long _movingTimeMs = 0;
    unsigned long _elapsedTimeMs = 0;

    unsigned long _lastUpdateMs = 0;
    const float _moveThresholdKmh = 3.0f;  // 「移動中」とみなすための閾値

   public:
    TripComputer() {}

    void begin() {
        reset();
        _lastUpdateMs = millis();
    }

    void update(float currentSpeedKmh, unsigned long currentMs) {
        if (_lastUpdateMs == 0) {
            _lastUpdateMs = currentMs;
            return;
        }

        unsigned long deltaMs = currentMs - _lastUpdateMs;
        _lastUpdateMs = currentMs;

        // 常に合計経過時間を更新
        _elapsedTimeMs += deltaMs;

        // 最大速度を更新
        if (currentSpeedKmh > _maxSpeedKmh) {
            _maxSpeedKmh = currentSpeedKmh;
        }

        // 距離と時間の計算
        // 単純な積分: 距離 = 速度 * 時間
        // 速度は km/h、時間は ms。
        // 距離 (km) = 速度 (km/h) * (deltaMs / 1000.0 / 3600.0)

        if (currentSpeedKmh > _moveThresholdKmh) {
            // 移動時間を加算
            _movingTimeMs += deltaMs;

            // 距離を加算
            double hours = (double)deltaMs / 3600000.0;
            _totalDistanceKm += (double)currentSpeedKmh * hours;
        }
    }

    void reset() {
        _maxSpeedKmh = 0.0f;
        _totalDistanceKm = 0.0;
        _movingTimeMs = 0;
        _elapsedTimeMs = 0;
        _lastUpdateMs = millis();
    }

    float getMaxSpeedKmh() {
        return _maxSpeedKmh;
    }

    float getDistanceKm() {
        return (float)_totalDistanceKm;
    }

    // ms を文字列にフォーマットするヘルパー
    void msToTimeStr(unsigned long ms, char* buffer, size_t size) {
        unsigned long totalSeconds = ms / 1000;
        unsigned long hours = totalSeconds / 3600;
        unsigned long minutes = (totalSeconds % 3600) / 60;
        unsigned long seconds = totalSeconds % 60;

        snprintf(buffer, size, "%02lu:%02lu:%02lu", hours, minutes, seconds);
    }

    // 移動時間を "HH:MM:SS" 形式で返す
    void getMovingTimeStr(char* buffer, size_t size) {
        msToTimeStr(_movingTimeMs, buffer, size);
    }

    // 合計経過時間を "HH:MM:SS" 形式で返す
    void getElapsedTimeStr(char* buffer, size_t size) {
        msToTimeStr(_elapsedTimeMs, buffer, size);
    }

    float getAvgSpeedKmh() {
        if (_movingTimeMs == 0) return 0.0f;

        double hours = (double)_movingTimeMs / 3600000.0;
        return (float)(_totalDistanceKm / hours);
    }
};

#endif
