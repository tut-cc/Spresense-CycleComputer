#include "GPSWrapper.h"

#include <Arduino.h>

#include "../Config.h"
#include "../system/Logger.h"

bool GPSWrapper::begin() {
    if (gnss.begin() != 0) {
        LOG_ERROR("GPS begin failed");
        return false;
    }

    gnss.select(GPS);
    gnss.select(QZ_L1CA);

    if (gnss.start(COLD_START) != 0) {
        LOG_ERROR("GPS start failed");
        return false;
    }

    LOG_DEBUG_IF(Config::Debug::ENABLE_GPS_LOG, "GPS initialized successfully");
    return true;
}

void GPSWrapper::update() {
    if (!gnss.waitUpdate(0)) return;
    gnss.getNavData(&navData);

    if (navData.posFixMode == Fix2D || navData.posFixMode == Fix3D) {
        LOG_DEBUG_IF(Config::Debug::ENABLE_GPS_LOG, "GPS Fix: Mode=%d, Sats=%d, Lat=%f, Lon=%f, Vel=%f, Time=%02d:%02d:%02d", 
           navData.posFixMode, 
           navData.numSatellites,
           navData.latitude,
           navData.longitude,
           navData.velocity,
           navData.time.hour, navData.time.minute, navData.time.second
        );
    }
}

float GPSWrapper::getSpeedKmh() const {
    if (!(navData.posFixMode == Fix2D || navData.posFixMode == Fix3D)) {
        return 0.0f;
    }
    if (navData.velocity < 0.1f) return 0.0f;  // 測位誤差対策
    
    float speedKmh = navData.velocity * 3.6f;
    return speedKmh;
}

void GPSWrapper::getTimeJST(char* displayData, size_t size) const {
    if (!(navData.time.year >= 2020)) {
        snprintf(displayData, size, "??:??");
        return;
    }

    int hour = (navData.time.hour + Config::Time::JST_OFFSET + 24) % 24;
    int minute = navData.time.minute;
    snprintf(displayData, size, "%02d:%02d", hour, minute);
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
