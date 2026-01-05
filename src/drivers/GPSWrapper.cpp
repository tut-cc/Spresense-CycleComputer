#include "GPSWrapper.h"

#include <Arduino.h>

GPSWrapper::GPSWrapper() : isFixed(false) {
#ifndef IS_SPRESENSE
    mockSpeed = 0.0;
    lastUpdate = 0;
    mockHour = 12;
    mockMinute = 0;
    mockSecond = 0;
#endif
}

bool GPSWrapper::begin() {
#ifdef IS_SPRESENSE
    int ret;
    ret = gnss.begin();
    if (ret != 0) {
        return false;
    }

    // GPSとQZSSのみを選択（日本地域向けに最適化）
    gnss.select(GPS);
    gnss.select(QZ_L1CA);

    ret = gnss.start(COLD_START);
    if (ret != 0) {
        return false;
    }

    return true;
#else
    // モックの初期化は常に成功する
    isFixed = true;
    return true;
#endif
}

void GPSWrapper::update() {
#ifdef IS_SPRESENSE
    // 可能であればノンブロッキングチェック、または短いタイムアウト
    // データがない場合に即座に戻るためにタイムアウトとして 0 を使用
    if (gnss.waitUpdate(0)) {
        gnss.getNavData(&navData);
        // 位置が特定されたかを確認
        isFixed = (navData.posFixMode >= Fix2D);
    }
#else
    // モック更新: 時間を進め、速度をシミュレート
    unsigned long current = millis();
    if (current - lastUpdate >= 1000) {
        lastUpdate = current;

        // 時間シミュレーション
        mockSecond++;
        if (mockSecond >= 60) {
            mockSecond = 0;
            mockMinute++;
            if (mockMinute >= 60) {
                mockMinute = 0;
                mockHour++;
                if (mockHour >= 24) mockHour = 0;
            }
        }

        // 速度シミュレーション (0 から 30 km/h の正弦波)
        mockSpeed = 15.0 + 15.0 * sin(current / 10000.0);
    }
#endif
}

float GPSWrapper::getSpeedKmh() {
#ifdef IS_SPRESENSE
    if (!isFixed) {
        return 0.0f;
    }
    // 速度は m/s 単位、km/h に変換
    return navData.velocity * 3.6f;
#else
    return mockSpeed;
#endif
}

void GPSWrapper::getTimeJST(char *buffer, size_t size) {
#ifdef IS_SPRESENSE
    if (navData.time.year == 0) {
        snprintf(buffer, size, "00:00");
        return;
    }

    // 単純な UTC から JST への変換 (+9 時間)
    // 表示用の時間のみを処理 (日付の繰り越しは無視)
    int hour = navData.time.hour + 9;
    if (hour >= 24) {
        hour -= 24;
    }
    int minute = navData.time.minute;

    snprintf(buffer, size, "%02d:%02d", hour, minute);
#else
    snprintf(buffer, size, "%02d:%02d", mockHour, mockMinute);
#endif
}
