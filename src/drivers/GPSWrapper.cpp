#include "GPSWrapper.h"

#include <Arduino.h>
#include "../Config.h"

bool GPSWrapper::begin() {
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
}

void GPSWrapper::update() {
    // 可能であればノンブロッキングチェック、または短いタイムアウト
    // データがない場合に即座に戻るためにタイムアウトとして 0 を使用
    if (gnss.waitUpdate(0)) {
        gnss.getNavData(&navData);
        // 位置が特定されたかを確認
        isFixed = (navData.posFixMode >= Fix2D);
    }
}

float GPSWrapper::getSpeedKmh() {
    if (!isFixed) return 0.0f;
    // 速度は m/s 単位、km/h に変換
    return navData.velocity * 3.6f;
}

void GPSWrapper::getTimeJST(char *buffer, size_t size) {
    if (navData.time.year == 0) {
        snprintf(buffer, size, "00:00");
        return;
    }

    // 単純な UTC から JST への変換
    // 表示用の時間のみを処理 (日付の繰り越しは無視)
    int hour = navData.time.hour + Config::Time::TIMEZONE_OFFSET;
    if (hour >= 24) hour -= 24;
    int minute = navData.time.minute;

    snprintf(buffer, size, "%02d:%02d", hour, minute);
}
