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
    // データがない場合に即座に戻るためにタイムアウトとして 0 を使用
    if (gnss.waitUpdate(0)) {
        gnss.getNavData(&navData);
        // 位置が特定されたかを確認
        isFixed = (navData.posFixMode >= Fix2D);
    }
}

float GPSWrapper::getSpeedKmh() {
    if (!isFixed) return 0.0f;
    return navData.velocity * 3.6f;  // 速度は m/s 単位、km/h に変換
}

void GPSWrapper::getTimeJST(char *buffer, size_t size) {
    if (navData.time.year == 0) {
        snprintf(buffer, size, "00:00");
        return;
    }

    // 単純な UTC から JST への変換
    int hour = navData.time.hour + Config::Time::TIMEZONE_OFFSET;
    if (hour >= 24) hour -= 24;
    int minute = navData.time.minute;

    snprintf(buffer, size, "%02d:%02d", hour, minute);
}

#ifdef UNIT_TEST
void GPSWrapper::setMockTime(int h, int m, int s) {
    SpGnss::mockTimeData.hour = h;
    SpGnss::mockTimeData.minute = m;
    SpGnss::mockTimeData.second = s;
}

void GPSWrapper::setMockSpeed(float speedKmh) {
    SpGnss::mockVelocityData = speedKmh / 3.6f;
}
#endif
